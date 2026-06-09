#include "Achievements.h"
#include <sstream>
#include <algorithm>

AchievementSystem::AchievementSystem() {
    achievements["first_exam"] = {"first_exam", "Первый экзамен", "Сдать первый экзамен"};
    achievements["excellent"] = {"excellent", "Отличник", "Получить 5 на любом экзамене"};
    achievements["all_exams"] = {"all_exams", "Красный диплом", "Сдать все экзамены на 4+", false};
    achievements["millionaire"] = {"millionaire", "Миллионер", "Накопить 5000 рублей"};
    achievements["friend"] = {"friend", "Друг", "Подружиться с NPC (отношения 80+)"};
    achievements["romance"] = {"romance", "Романтика", "Построить романтические отношения"};
    achievements["survivor"] = {"survivor", "Выживальщик", "Дожить до конца игры"};
    achievements["helper"] = {"helper", "Помощник", "Помочь NPC 5 раз"};
    achievements["hermit"] = {"hermit", "Отшельник", "Провести день не выходя из дома"};
    achievements["sportsman"] = {"sportsman", "Спортсмен", "Сходить на физкультуру 3 раза"};
    achievements["bookworm"] = {"bookworm", "Книжный червь", "Прочитать 3 книги в библиотеке"};
    achievements["noodle"] = {"noodle", "Доширак", "Съесть 5 дошираков"};
    achievements["surprise"] = {"surprise", "Сюрприз!", "Найти секретное событие"};
}

bool AchievementSystem::unlock(const std::string& id) {
    auto it = achievements.find(id);
    if (it != achievements.end() && !it->second.unlocked) {
        it->second.unlocked = true;
        return true;
    }
    return false;
}

bool AchievementSystem::isUnlocked(const std::string& id) const {
    auto it = achievements.find(id);
    return it != achievements.end() && it->second.unlocked;
}

const Achievement* AchievementSystem::getAchievement(const std::string& id) const {
    auto it = achievements.find(id);
    if (it != achievements.end()) return &it->second;
    return nullptr;
}

std::vector<const Achievement*> AchievementSystem::getUnlocked() const {
    std::vector<const Achievement*> result;
    for (const auto& [id, ach] : achievements) {
        if (ach.unlocked) result.push_back(&ach);
    }
    return result;
}

std::vector<const Achievement*> AchievementSystem::getAll() const {
    std::vector<const Achievement*> result;
    for (const auto& [id, ach] : achievements) {
        result.push_back(&ach);
    }
    return result;
}

int AchievementSystem::getUnlockedCount() const {
    int count = 0;
    for (const auto& [id, ach] : achievements) {
        if (ach.unlocked) count++;
    }
    return count;
}

int AchievementSystem::getTotalCount() const {
    return static_cast<int>(achievements.size());
}

std::string AchievementSystem::serialize() const {
    std::ostringstream oss;
    oss << achievements.size() << "\n";
    for (const auto& [id, ach] : achievements) {
        oss << id << " " << (ach.unlocked ? "1" : "0") << "\n";
    }
    return oss.str();
}

bool AchievementSystem::deserialize(const std::string& data) {
    std::istringstream iss(data);
    size_t count;
    if (!(iss >> count)) return false;
    for (size_t i = 0; i < count; i++) {
        std::string id;
        int val;
        if (!(iss >> id >> val)) return false;
        auto it = achievements.find(id);
        if (it != achievements.end()) {
            it->second.unlocked = (val == 1);
        }
    }
    return true;
}
