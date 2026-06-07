#include "NPC.h"
#include "../player/Player.h"
#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {
    bool isPositiveCategory(const std::string& category) {
        return category == "helpful" ||
               category == "generous" ||
               category == "romance" ||
               category == "trust" ||
               category == "honest";
    }

    bool isNegativeCategory(const std::string& category) {
        return category == "rude" ||
               category == "selfish" ||
               category == "pushy";
    }

    std::vector<std::string> splitLine(const std::string& line, char delimiter) {
        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string part;

        while (std::getline(ss, part, delimiter)) {
            parts.push_back(part);
        }

        return parts;
    }

    int safeToInt(const std::string& value, int fallback = 0) {
        try {
            size_t used = 0;
            int result = std::stoi(value, &used);
            if (used != value.size()) return fallback;
            return result;
        } catch (...) {
            return fallback;
        }
    }

    Choice makeChoice(
        const std::string& text,
        const std::vector<std::pair<ChoiceEffect, int>>& effects,
        const std::string& resultingText,
        int requiredDay = 0,
        int requiredRelation = 0,
        const std::string& requiredFlag = ""
    ) {
        Choice choice;
        choice.text = text;
        choice.effects = effects;
        choice.resultingText = resultingText;
        choice.requiredDay = requiredDay;
        choice.requiredRelation = requiredRelation;
        choice.requiredFlag = requiredFlag;
        return choice;
    }
}

NPC::NPC(const std::string& npcName, const std::string& desc)
    : name(npcName), description(desc) {
}

void NPC::modifyRelationship(int delta) {
    // Совместимость со старым кодом.
    // Игровые отношения живут в Player, а NPC меняет только доверие.
    modifyTrust(delta);
}

void NPC::modifyTrust(int delta) {
    trustLevel = std::clamp(trustLevel + delta, 0, 100);
}

void NPC::rememberEvent(const std::string& eventKey, int value) {
    memory[eventKey] = { eventKey, value, true };
}

bool NPC::hasMemory(const std::string& eventKey) const {
    auto it = memory.find(eventKey);
    return it != memory.end() && it->second.triggered;
}

int NPC::getMemoryValue(const std::string& eventKey) const {
    auto it = memory.find(eventKey);
    return it != memory.end() ? it->second.value : 0;
}

bool NPC::wasHelped() const {
    return hasMemory("helped_player") || getHelpCount() > 0;
}

bool NPC::wasRude() const {
    return hasMemory("was_rude");
}

int NPC::getHelpCount() const {
    return getMemoryValue("help_count");
}

void NPC::recordAction(const NPCAction& action) {
    actionHistory.push_back(action);

    // Здесь меняется только доверие NPC.
    // Реальные отношения для концовок и игровых проверок хранятся в Player.
    if (action.category == "helpful") {
        modifyTrust(action.influence / 2);
    } else if (action.category == "generous") {
        modifyTrust(action.influence / 2);
    } else if (action.category == "romance") {
        modifyTrust(action.influence);
    } else if (action.category == "trust") {
        modifyTrust(action.influence);
    } else if (action.category == "honest") {
        modifyTrust(std::max(1, action.influence / 2));
    } else if (action.category == "rude" || action.category == "selfish") {
        modifyTrust(-action.influence / 2);
    } else if (action.category == "pushy") {
        modifyTrust(-std::max(1, action.influence / 3));
    }

    if (!action.fulfilledPromise) {
        modifyTrust(-10);
    }
}

int NPC::countActionsByCategory(const std::string& category) const {
    int count = 0;
    for (const auto& action : actionHistory) {
        if (action.category == category) {
            count++;
        }
    }
    return count;
}

bool NPC::hasPositiveHistory(int currentDay, int recentDays) const {
    int posCount = 0;
    int negCount = 0;
    int minDay = currentDay - recentDays;

    for (const auto& action : actionHistory) {
        if (action.day < minDay || action.day > currentDay) {
            continue;
        }

        if (isPositiveCategory(action.category)) {
            posCount++;
        } else if (isNegativeCategory(action.category) || !action.fulfilledPromise) {
            negCount++;
        }
    }

    return posCount > negCount;
}

bool NPC::hasNegativeHistory(int currentDay, int recentDays) const {
    int posCount = 0;
    int negCount = 0;
    int minDay = currentDay - recentDays;

    for (const auto& action : actionHistory) {
        if (action.day < minDay || action.day > currentDay) {
            continue;
        }

        if (isPositiveCategory(action.category)) {
            posCount++;
        } else if (isNegativeCategory(action.category) || !action.fulfilledPromise) {
            negCount++;
        }
    }

    return negCount > posCount;
}

std::string NPC::getEmotionalState() const {
    if (trustLevel >= 80) return "доверчивое";
    if (trustLevel >= 60) return "тёплое";
    if (trustLevel >= 40) return "нейтральное";
    if (trustLevel >= 20) return "настороженное";
    return "закрытое";
}

std::string NPC::getDialog(const Player& player) const {
    int rel = player.getRelation(name);

    if (rel >= 80) return name + ": Рад(а) тебя видеть! Как у тебя дела?";
    if (rel >= 60) return name + ": Привет, " + player.getName() + "! Хорошо, что встретились.";
    if (rel >= 40) return name + ": Привет, " + player.getName() + ". Как жизнь?";
    if (rel >= 20) return name + ": Здравствуй...";
    return name + ": ... (отворачивается)";
}

std::vector<Choice> NPC::getChoices(const Player& player) const {
    (void)player;
    return {};
}

std::string NPC::serialize() const {
    std::string data;

    data += name + "\n";

    // Старый формат хранил отдельный relationship.
    // Оставляем строку для совместимости, но пишем туда trustLevel.
    data += std::to_string(trustLevel) + "\n";
    data += std::to_string(trustLevel) + "\n";

    data += std::to_string(memory.size()) + "\n";
    for (const auto& item : memory) {
        const auto& key = item.first;
        const auto& entry = item.second;
        data += key + "," + std::to_string(entry.value) + "," + (entry.triggered ? "1" : "0") + "\n";
    }

    data += "---ACTIONS---\n";
    data += std::to_string(actionHistory.size()) + "\n";
    for (const auto& action : actionHistory) {
        data += action.type + "|" +
                std::to_string(action.day) + "|" +
                std::to_string(action.influence) + "|" +
                action.location + "|" +
                action.emotion + "|" +
                action.result + "|" +
                action.category + "|" +
                (action.fulfilledPromise ? "1" : "0") + "\n";
    }

    return data;
}

bool NPC::deserialize(const std::string& data) {
    std::istringstream iss(data);

    std::string loadedName;
    if (!std::getline(iss, loadedName)) {
        return false;
    }

    if (!loadedName.empty()) {
        name = loadedName;
    }

    int legacyRelationship = trustLevel;
    int loadedTrust = trustLevel;
    size_t memSize = 0;

    // Формат сохранения:
    // name
    // legacy relationship
    // trustLevel
    // memorySize
    // ...
    if (!(iss >> legacyRelationship)) {
        return false;
    }

    if (!(iss >> loadedTrust)) {
        loadedTrust = legacyRelationship;
        iss.clear();
    }

    trustLevel = std::clamp(loadedTrust, 0, 100);

    if (!(iss >> memSize)) {
        return true;
    }

    iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    memory.clear();
    for (size_t i = 0; i < memSize; i++) {
        std::string line;
        if (!std::getline(iss, line)) {
            return false;
        }

        auto parts = splitLine(line, ',');
        if (parts.size() < 3) {
            continue;
        }

        std::string key = parts[0];
        int value = safeToInt(parts[1]);
        bool triggered = safeToInt(parts[2]) == 1;

        memory[key] = { key, value, triggered };
    }

    std::string marker;
    if (!std::getline(iss, marker)) {
        return true;
    }

    if (marker != "---ACTIONS---") {
        return true;
    }

    size_t actionSize = 0;
    if (!(iss >> actionSize)) {
        return true;
    }
    iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    actionHistory.clear();
    for (size_t i = 0; i < actionSize; i++) {
        std::string line;
        if (!std::getline(iss, line)) {
            break;
        }

        auto parts = splitLine(line, '|');
        if (parts.size() < 8) {
            continue;
        }

        NPCAction action;
        action.type = parts[0];
        action.day = safeToInt(parts[1]);
        action.influence = safeToInt(parts[2]);
        action.location = parts[3];
        action.emotion = parts[4];
        action.result = parts[5];
        action.category = parts[6];
        action.fulfilledPromise = parts[7] == "1";

        actionHistory.push_back(action);
    }

    return true;
}

// ---- Alla ----
Alla::Alla() : NPC("Алла", "Одногруппница Тимура, умная и красивая девушка") {}

std::string Alla::getDialog(const Player& player) const {
    int rel = player.getRelation("Алла");
    int trust = getTrust();

    if (hasMemory("trusted_player")) {
        trust += getMemoryValue("trusted_player");
    }

    if (rel >= 80 && trust > 53) {
        return "Алла сияет: «" + player.getName() + "! Ты мой самый любимый человек!»";
    }
    if (rel >= 60) {
        return "Алла тепло улыбается: «Привет! Я как раз думала о тебе.»";
    }
    if (rel >= 40) {
        return "Алла улыбается: «Рада тебя видеть, " + player.getName() + "! Как дела?»";
    }
    if (rel >= 20) {
        return "Алла: «Привет, " + player.getName() + ". Какими судьбами?»";
    }
    return "Алла холодно смотрит: «Нам не о чем говорить.»";
}

std::vector<Choice> Alla::getChoices(const Player& player) const {
    std::vector<Choice> choices;

    choices.push_back(makeChoice(
        "Поздороваться и поговорить",
        {},
        "Ты вежливо здороваешься с Аллой."
    ));

    choices.push_back(makeChoice(
        "Сделать комплимент",
        {{ChoiceEffect::IncreaseAllaRelation, 5}, {ChoiceEffect::IncreaseRomance, 3}},
        "Ты говоришь Алле, что она прекрасно выглядит."
    ));

    choices.push_back(makeChoice(
        "Предложить помощь",
        {{ChoiceEffect::IncreaseAllaRelation, 10}},
        "Ты предлагаешь Алле помощь."
    ));

    if (player.getRelation("Алла") >= 70 && player.hasFlag("day7_available")) {
        choices.push_back(makeChoice(
            "Пригласить на свидание",
            {{ChoiceEffect::IncreaseRomance, 10}, {ChoiceEffect::IncreaseAllaRelation, 15}},
            "Ты приглашаешь Аллу на свидание."
        ));
    }

    return choices;
}

// ---- Bulat ----
Bulat::Bulat() : NPC("Булат", "Лучший друг Тимура, весёлый и надёжный парень") {}

std::string Bulat::getDialog(const Player& player) const {
    int rel = player.getRelation("Булат");

    if (rel >= 70) {
        return "Булат широко улыбается: «Дружище " + player.getName() + "! Рад тебя видеть!»";
    }
    if (rel >= 40) {
        return "Булат хлопает по плечу: «" + player.getName() + ", братан! Давно не виделись!»";
    }
    if (rel >= 20) {
        return "Булат: «Здорово, " + player.getName() + "! Какие планы?»";
    }
    return "Булат отворачивается: «Не до тебя сейчас.»";
}

std::vector<Choice> Bulat::getChoices(const Player& player) const {
    (void)player;

    std::vector<Choice> choices;

    choices.push_back(makeChoice(
        "Поболтать",
        {},
        "Вы обмениваетесь новостями."
    ));

    choices.push_back(makeChoice(
        "Попросить помощи с учёбой",
        {{ChoiceEffect::IncreaseIntellect, 5}, {ChoiceEffect::IncreaseBulatRelation, 5}},
        "Булат помогает тебе разобраться с материалом."
    ));

    choices.push_back(makeChoice(
        "Предложить сходить в столовую",
        {{ChoiceEffect::IncreaseBulatRelation, 3}},
        "Вы идёте в столовую вместе."
    ));

    return choices;
}

// ---- Semen ----
Semen::Semen() : NPC("Семён", "Студент-старшекурсник, циничный и умный") {}

std::string Semen::getDialog(const Player& player) const {
    int rel = player.getRelation("Семён");

    if (rel >= 60) {
        return "Семён: «Привет, " + player.getName() + "! Как успехи? Я могу помочь с учёбой.»";
    }
    if (rel >= 30) {
        return "Семён: «Привет, " + player.getName() + ". Как успехи в учёбе?»";
    }
    return "Семён сухо кивает: «" + player.getName() + ". Чего надо?»";
}

std::vector<Choice> Semen::getChoices(const Player& player) const {
    (void)player;

    std::vector<Choice> choices;

    choices.push_back(makeChoice(
        "Спросить про учёбу",
        {},
        "Семён даёт несколько советов."
    ));

    choices.push_back(makeChoice(
        "Попросить конспекты",
        {{ChoiceEffect::IncreaseIntellect, 8}, {ChoiceEffect::IncreaseSemenRelation, 3}},
        "Семён даёт свои конспекты."
    ));

    return choices;
}

// ---- Artem ----
Artem::Artem() : NPC("Артём", "Замкнутый программист, гений математики") {}

std::string Artem::getDialog(const Player& player) const {
    int rel = player.getRelation("Артём");

    if (rel >= 60) {
        return "Артём: «" + player.getName() + "! Отличная встреча! Есть новая задача — хочешь решить?»";
    }
    if (rel >= 30) {
        return "Артём: «О, " + player.getName() + "! Как тебе новый материал по матанализу?»";
    }
    return "Артём не поднимает взгляда от ноутбука: «А, привет.»";
}

std::vector<Choice> Artem::getChoices(const Player& player) const {
    (void)player;

    std::vector<Choice> choices;

    choices.push_back(makeChoice(
        "Поговорить о программировании",
        {{ChoiceEffect::IncreaseIntellect, 5}},
        "Вы обсуждаете алгоритмы."
    ));

    choices.push_back(makeChoice(
        "Попросить помочь с математикой",
        {{ChoiceEffect::IncreaseIntellect, 10}},
        "Артём объясняет сложную тему."
    ));

    return choices;
}