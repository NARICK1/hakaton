#include "NPC.h"
#include "../player/Player.h"
#include <algorithm>
#include <sstream>

NPC::NPC(const std::string& npcName, const std::string& desc)
    : name(npcName), description(desc) {
}

void NPC::modifyRelationship(int delta) {
    relationship = std::clamp(relationship + delta, 0, 100);
}

void NPC::modifyTrust(int delta) {
    trustLevel = std::clamp(trustLevel + delta, 0, 100);
}

void NPC::rememberEvent(const std::string& eventKey, int value) {
    memory[eventKey] = { eventKey, value, true };
}

bool NPC::hasMemory(const std::string& eventKey) const {
    return memory.find(eventKey) != memory.end() && memory.at(eventKey).triggered;
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
    // Auto-update trust and relationship based on categories
    if (action.category == "helpful") {
        modifyRelationship(action.influence);
        modifyTrust(action.influence / 2);
    } else if (action.category == "rude" || action.category == "selfish") {
        modifyRelationship(-action.influence);
        modifyTrust(-action.influence / 2);
    } else if (action.category == "romance") {
        modifyRelationship(action.influence * 2);
    } else if (action.category == "trust") {
        modifyTrust(action.influence);
        modifyRelationship(action.influence);
    }
    if (!action.fulfilledPromise) {
        modifyTrust(-10);
        modifyRelationship(-5);
    }
}

int NPC::countActionsByCategory(const std::string& category) const {
    int count = 0;
    for (const auto& a : actionHistory) {
        if (a.category == category) count++;
    }
    return count;
}

bool NPC::hasPositiveHistory(int recentDays) const {
    int currentDay = 0; // placeholder, caller's context would set this
    int posCount = 0, negCount = 0;
    for (const auto& a : actionHistory) {
        if (a.day >= currentDay - recentDays) {
            if (a.category == "helpful" || a.category == "generous" || a.category == "romance" || a.category == "trust")
                posCount++;
            else if (a.category == "rude" || a.category == "selfish")
                negCount++;
        }
    }
    return posCount > negCount;
}

bool NPC::hasNegativeHistory(int recentDays) const {
    int currentDay = 0;
    int posCount = 0, negCount = 0;
    for (const auto& a : actionHistory) {
        if (a.day >= currentDay - recentDays) {
            if (a.category == "helpful" || a.category == "generous" || a.category == "romance" || a.category == "trust")
                posCount++;
            else if (a.category == "rude" || a.category == "selfish")
                negCount++;
        }
    }
    return negCount > posCount;
}

std::string NPC::getEmotionalState() const {
    if (relationship >= 80) return "восторженное";
    if (relationship >= 60) return "тёплое";
    if (relationship >= 40) return "нейтральное";
    if (relationship >= 20) return "холодное";
    return "враждебное";
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
    return {};
}

std::string NPC::serialize() const {
    std::string data = name + "\n" + std::to_string(relationship) + "\n";
    data += std::to_string(trustLevel) + "\n";
    data += std::to_string(memory.size()) + "\n";
    for (const auto& [key, entry] : memory) {
        data += key + "," + std::to_string(entry.value) + "," + (entry.triggered ? "1" : "0") + "\n";
    }
    data += "---ACTIONS---\n";
    data += std::to_string(actionHistory.size()) + "\n";
    for (const auto& a : actionHistory) {
        data += a.type + "|" + std::to_string(a.day) + "|" + std::to_string(a.influence) + "|"
              + a.location + "|" + a.emotion + "|" + a.result + "|" + a.category + "|"
              + (a.fulfilledPromise ? "1" : "0") + "\n";
    }
    return data;
}

bool NPC::deserialize(const std::string& data) {
    std::istringstream iss(data);
    if (!std::getline(iss, name)) return false;
    if (!(iss >> relationship)) return false;
    if (!(iss >> trustLevel)) return false;
    size_t memSize;
    if (!(iss >> memSize)) return false;
    memory.clear();
    for (size_t i = 0; i < memSize; i++) {
        std::string key;
        int val, trig;
        if (!std::getline(iss >> std::ws, key, ',')) return false;
        if (!(iss >> val)) return false;
        iss.ignore(1);
        if (!(iss >> trig)) return false;
        iss.ignore(1); // skip newline
        memory[key] = { key, val, trig == 1 };
    }
    std::string marker;
    iss >> marker;
    if (marker != "---ACTIONS---") return true; // backward compat
    size_t actSize;
    if (!(iss >> actSize)) return true;
    iss.ignore();
    actionHistory.clear();
    for (size_t i = 0; i < actSize; i++) {
        NPCAction a;
        std::getline(iss, a.type, '|');
        std::string d, inf, fp;
        std::getline(iss, d, '|'); a.day = std::stoi(d);
        std::getline(iss, inf, '|'); a.influence = std::stoi(inf);
        std::getline(iss, a.location, '|');
        std::getline(iss, a.emotion, '|');
        std::getline(iss, a.result, '|');
        std::getline(iss, a.category, '|');
        std::getline(iss, fp); a.fulfilledPromise = (fp == "1");
        actionHistory.push_back(a);
    }
    return true;
}

// ---- Alla ----
Alla::Alla() : NPC("Алла", "Одногруппница Тимура, умная и красивая девушка") {}

std::string Alla::getDialog(const Player& player) const {
    int rel = player.getRelation("Алла");
    int trust = 0;
    if (hasMemory("trusted_player")) trust = getMemoryValue("trusted_player");
    if (rel >= 80 && trust > 3)
        return "Алла сияет: «" + player.getName() + "! Ты мой самый любимый человек!»";
    if (rel >= 60)
        return "Алла тепло улыбается: «Привет! Я как раз думала о тебе.»";
    if (rel >= 40)
        return "Алла улыбается: «Рада тебя видеть, " + player.getName() + "! Как дела?»";
    if (rel >= 20)
        return "Алла: «Привет, " + player.getName() + ". Какими судьбами?»";
    return "Алла холодно смотрит: «Нам не о чем говорить.»";
}

std::vector<Choice> Alla::getChoices(const Player& player) const {
    std::vector<Choice> choices;
    choices.push_back({ "Поздороваться и поговорить", {}, "Ты вежливо здороваешься с Аллой." });
    choices.push_back({ "Сделать комплимент",
        {{ChoiceEffect::IncreaseAllaRelation, 5}, {ChoiceEffect::IncreaseRomance, 3}},
        "Ты говоришь Алле, что она прекрасно выглядит." });
    choices.push_back({ "Предложить помощь",
        {{ChoiceEffect::IncreaseAllaRelation, 10}},
        "Ты предлагаешь Алле помощь." });
    if (player.getRelation("Алла") >= 70 && player.hasFlag("day7_available")) {
        choices.push_back({ "Пригласить на свидание",
            {{ChoiceEffect::IncreaseRomance, 10}, {ChoiceEffect::IncreaseAllaRelation, 15}},
            "Ты приглашаешь Аллу на свидание." });
    }
    return choices;
}

// ---- Bulat ----
Bulat::Bulat() : NPC("Булат", "Лучший друг Тимура, весёлый и надёжный парень") {}

std::string Bulat::getDialog(const Player& player) const {
    int rel = player.getRelation("Булат");
    if (rel >= 70)
        return "Булат широко улыбается: «Дружище " + player.getName() + "! Рад тебя видеть!»";
    if (rel >= 40)
        return "Булат хлопает по плечу: «" + player.getName() + ", братан! Давно не виделись!»";
    if (rel >= 20)
        return "Булат: «Здорово, " + player.getName() + "! Какие планы?»";
    return "Булат отворачивается: «Не до тебя сейчас.»";
}

std::vector<Choice> Bulat::getChoices(const Player& player) const {
    std::vector<Choice> choices;
    choices.push_back({ "Поболтать", {}, "Вы обмениваетесь новостями." });
    choices.push_back({ "Попросить помощи с учёбой",
        {{ChoiceEffect::IncreaseIntellect, 5}, {ChoiceEffect::IncreaseBulatRelation, 5}},
        "Булат помогает тебе разобраться с материалом." });
    choices.push_back({ "Предложить сходить в столовую",
        {{ChoiceEffect::IncreaseBulatRelation, 3}},
        "Вы идёте в столовую вместе." });
    return choices;
}

// ---- Semen ----
Semen::Semen() : NPC("Семён", "Студент-старшекурсник, циничный и умный") {}

std::string Semen::getDialog(const Player& player) const {
    int rel = player.getRelation("Семён");
    if (rel >= 60)
        return "Семён: «Привет, " + player.getName() + "! Как успехи? Я могу помочь с учёбой.»";
    if (rel >= 30)
        return "Семён: «Привет, " + player.getName() + ". Как успехи в учёбе?»";
    return "Семён сухо кивает: «" + player.getName() + ". Чего надо?»";
}

std::vector<Choice> Semen::getChoices(const Player& player) const {
    std::vector<Choice> choices;
    choices.push_back({ "Спросить про учёбу", {}, "Семён даёт несколько советов." });
    choices.push_back({ "Попросить конспекты",
        {{ChoiceEffect::IncreaseIntellect, 8}, {ChoiceEffect::IncreaseSemenRelation, 3}},
        "Семён даёт свои конспекты." });
    return choices;
}

// ---- Artem ----
Artem::Artem() : NPC("Артём", "Замкнутый программист, гений математики") {}

std::string Artem::getDialog(const Player& player) const {
    int rel = player.getRelation("Артём");
    if (rel >= 60)
        return "Артём: «" + player.getName() + "! Отличная встреча! Есть новая задача — хочешь решить?»";
    if (rel >= 30)
        return "Артём: «О, " + player.getName() + "! Как тебе новый материал по матанализу?»";
    return "Артём не поднимает взгляда от ноутбука: «А, привет.»";
}

std::vector<Choice> Artem::getChoices(const Player& player) const {
    std::vector<Choice> choices;
    choices.push_back({ "Поговорить о программировании",
        {{ChoiceEffect::IncreaseIntellect, 5}},
        "Вы обсуждаете алгоритмы." });
    choices.push_back({ "Попросить помочь с математикой",
        {{ChoiceEffect::IncreaseIntellect, 10}},
        "Артём объясняет сложную тему." });
    return choices;
}
