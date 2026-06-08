#include "Player.h"
#include <sstream>
#include <algorithm>

void Player::setDifficulty(DifficultyLevel value) {
    difficulty = value;
}

Player::Player(const std::string& playerName)
    : name(playerName) {
    npcRelationships["Алла"] = GameConstants::START_RELATIONSHIP;
    npcRelationships["Булат"] = GameConstants::START_RELATIONSHIP;
    npcRelationships["Семён"] = GameConstants::START_RELATIONSHIP;
    npcRelationships["Артём"] = GameConstants::START_RELATIONSHIP;
    npcRelationships["Преподаватели"] = GameConstants::START_RELATIONSHIP;
}

int Player::getGrade(int examId) const {
    auto it = grades.find(examId);
    return it != grades.end() ? it->second : 0;
}

void Player::setGrade(int examId, int grade) {
    grades[examId] = grade;
}

void Player::addDebt(int count) {
    debts += count;
}

void Player::removeDebt(int count) {
    debts = std::max(0, debts - count);
}

void Player::nextDay() {
    currentDay++;
    currentHour = GameConstants::START_HOUR;
    currentMinute = 0;
    currentLocation = LocationID::Home;
    stats.money += scaleMoneyGain(GameConstants::MONEY_PER_DAY_SCHOLARSHIP);

    stats.hunger -= scalePenalty(GameConstants::DAY_HUNGER_LOSS);
    stats.fatigue += scalePenalty(5);
    stats.stress += scalePenalty(3);
    stats.clampAll();
}

void Player::advanceTime(int minutes) {
    currentMinute += minutes;
    while (currentMinute >= GameConstants::MINUTES_PER_HOUR) {
        currentMinute -= GameConstants::MINUTES_PER_HOUR;
        currentHour++;
    }
    if (currentHour >= GameConstants::HOURS_PER_DAY) {
        currentHour -= GameConstants::HOURS_PER_DAY;
        nextDay();
    }
    stats.hunger -= scalePenalty((minutes / 30) * GameConstants::TIME_HUNGER_LOSS_PER_30_MIN);
    stats.fatigue += scalePenalty(minutes / 60);
    stats.clampAll();
}

std::string Player::getTimeString() const {
    std::ostringstream oss;
    oss.width(2);
    oss.fill('0');
    oss << currentHour << ":";
    oss.width(2);
    oss.fill('0');
    oss << currentMinute;
    return oss.str();
}

void Player::setFlag(const std::string& key, bool value) {
    flags[key] = value;
}

bool Player::hasFlag(const std::string& key) const {
    auto it = flags.find(key);
    return it != flags.end() && it->second;
}

int Player::getRelation(const std::string& npcName) const {
    auto it = npcRelationships.find(npcName);
    return it != npcRelationships.end() ? it->second : GameConstants::START_RELATIONSHIP;
}

void Player::modifyRelation(const std::string& npcName, int delta) {
    auto it = npcRelationships.find(npcName);
    if (it != npcRelationships.end()) {
        int scaledDelta = delta;
        if (delta > 0) {
            scaledDelta = scaleGain(delta);
        } else if (delta < 0) {
            scaledDelta = -scalePenalty(-delta);
        }
        it->second = std::clamp(it->second + scaledDelta, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
    }
}

void Player::addBuff(BuffType buff) {
    if (!hasBuff(buff)) {
        activeBuffs.push_back(buff);
    }
}

void Player::removeBuff(BuffType buff) {
    activeBuffs.erase(
        std::remove(activeBuffs.begin(), activeBuffs.end(), buff),
        activeBuffs.end()
    );
}

bool Player::hasBuff(BuffType buff) const {
    return std::find(activeBuffs.begin(), activeBuffs.end(), buff) != activeBuffs.end();
}

void Player::applyBuffs() {
    if (hasBuff(BuffType::ImposterSyndrome)) {
    }
    if (hasBuff(BuffType::Burnout)) {
        stats.energy = std::min(stats.energy, 50);
    }
    if (hasBuff(BuffType::BrokenHeart)) {
        stats.stress += 1;
    }
    if (hasBuff(BuffType::SleepParalysis)) {
    }
    if (hasBuff(BuffType::Starvation)) {
    }
    stats.clampAll();
}

std::string Player::serialize() const {
    std::ostringstream oss;
    oss << name << "\n"
        << stats.intellect << " " << stats.energy << " "
        << stats.fatigue << " " << stats.hunger << " "
        << stats.stress << " " << stats.humanity << " "
        << stats.money << " " << stats.romance << " "
        << stats.health << " "
        << stats.confidence << " " << stats.burnout << " "
        << stats.motivation << " " << stats.anxiety << " "
        << stats.selfEsteem << "\n"
        << currentDay << " " << currentHour << " " << currentMinute << "\n"
        << static_cast<int>(currentLocation) << "\n"
        << debts << "\n";

    // Сохраняем оценки
    oss << grades.size() << "\n";
    for (const auto& [examId, grade] : grades) {
        oss << examId << " " << grade << "\n";
    }

    // Сохраняем флаги
    oss << flags.size() << "\n";
    for (const auto& [key, val] : flags) {
        oss << key << " " << (val ? "1" : "0") << "\n";
    }

    // Сохраняем отношения
    oss << npcRelationships.size() << "\n";
    for (const auto& [npc, rel] : npcRelationships) {
        oss << npc << " " << rel << "\n";
    }

    // Сохраняем активные баффы
    oss << activeBuffs.size() << "\n";
    for (auto b : activeBuffs) {
        oss << static_cast<int>(b) << "\n";
    }

    oss << static_cast<int>(difficulty) << "\n";

    return oss.str();
}

bool Player::deserialize(const std::string& data) {
    std::istringstream iss(data);
    if (!std::getline(iss, name)) return false;

    if (!(iss >> stats.intellect >> stats.energy >> stats.fatigue
        >> stats.hunger >> stats.stress >> stats.humanity
        >> stats.money >> stats.romance >> stats.health
        >> stats.confidence >> stats.burnout
        >> stats.motivation >> stats.anxiety >> stats.selfEsteem)) {
        // Backward compat: set defaults
        stats.confidence = 50; stats.burnout = 10;
        stats.motivation = 50; stats.anxiety = 30; stats.selfEsteem = 50;
    }

    int locInt;
    if (!(iss >> currentDay >> currentHour >> currentMinute >> locInt >> debts)) return false;
    currentLocation = static_cast<LocationID>(locInt);

    size_t gradeCount;
    if (!(iss >> gradeCount)) return false;
    grades.clear();
    for (size_t i = 0; i < gradeCount; i++) {
        int examId, grade;
        if (!(iss >> examId >> grade)) return false;
        grades[examId] = grade;
    }

    size_t flagCount;
    if (!(iss >> flagCount)) return false;
    flags.clear();
    for (size_t i = 0; i < flagCount; i++) {
        std::string key;
        int val;
        if (!(iss >> key >> val)) return false;
        flags[key] = (val == 1);
    }

    size_t relCount;
    if (!(iss >> relCount)) return false;
    npcRelationships.clear();
    for (size_t i = 0; i < relCount; i++) {
        std::string npc;
        int rel;
        if (!(iss >> npc >> rel)) return false;
        npcRelationships[npc] = rel;
    }

    // Загружаем активные баффы
    activeBuffs.clear();
    size_t buffCount;
    if (!(iss >> buffCount)) return false;
    for (size_t i = 0; i < buffCount; i++) {
        int b;
        if (!(iss >> b)) return false;
        activeBuffs.push_back(static_cast<BuffType>(b));
    }

    int difficultyInt = static_cast<int>(DifficultyLevel::Normal);
    if (iss >> difficultyInt) {
        if (difficultyInt == static_cast<int>(DifficultyLevel::Easy)) {
            difficulty = DifficultyLevel::Easy;
        } else if (difficultyInt == static_cast<int>(DifficultyLevel::Hard)) {
            difficulty = DifficultyLevel::Hard;
        } else {
            difficulty = DifficultyLevel::Normal;
        }
    } else {
        difficulty = DifficultyLevel::Normal;
    }

    stats.clampAll();
    return true;
}
