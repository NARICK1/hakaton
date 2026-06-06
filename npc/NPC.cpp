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

std::string NPC::getDialog(const Player& player) const {
    return name + ": Привет, " + player.getName() + "!";
}

std::vector<Choice> NPC::getChoices(const Player& player) const {
    return {};
}

std::string NPC::serialize() const {
    std::string data = name + "\n" + std::to_string(relationship) + "\n";
    data += std::to_string(memory.size()) + "\n";
    for (const auto& [key, entry] : memory) {
        data += key + " " + std::to_string(entry.value) + " " + (entry.triggered ? "1" : "0") + "\n";
    }
    return data;
}

bool NPC::deserialize(const std::string& data) {
    std::istringstream iss(data);
    if (!std::getline(iss, name)) return false;
    if (!(iss >> relationship)) return false;
    size_t memSize;
    if (!(iss >> memSize)) return false;
    memory.clear();
    for (size_t i = 0; i < memSize; i++) {
        std::string key;
        int val, trig;
        if (!(iss >> key >> val >> trig)) return false;
        memory[key] = { key, val, trig == 1 };
    }
    return true;
}

// ---- Alla ----
Alla::Alla() : NPC("Алла", "Одногруппница Тимура, умная и красивая девушка") {}

std::string Alla::getDialog(const Player& player) const {
    if (relationship < 20) {
        return "Алла холодно смотрит на тебя: «Нам не о чем говорить.»";
    } else if (relationship < 40) {
        return "Алла: «Привет, " + player.getName() + ". Какими судьбами?»";
    } else if (relationship < 60) {
        return "Алла улыбается: «Рада тебя видеть, " + player.getName() + "! Как дела?»";
    } else if (relationship < 80) {
        return "Алла тепло улыбается: «Привет! Я как раз думала о тебе.»";
    } else {
        return "Алла сияет: «" + player.getName() + "! Ты мой самый любимый человек!»";
    }
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
    if (relationship < 20) {
        return "Булат отворачивается: «Не до тебя сейчас.»";
    } else if (relationship < 40) {
        return "Булат: «Здорово! Какие планы?»";
    } else if (relationship < 60) {
        return "Булат хлопает тебя по плечу: «" + player.getName() + ", братан! Давно не виделись!»";
    } else {
        return "Булат широко улыбается: «Дружище! Рад тебя видеть!»";
    }
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
    if (relationship < 30) {
        return "Семён сухо кивает: «" + player.getName() + ". Чего надо?»";
    } else {
        return "Семён: «Привет, " + player.getName() + ". Как успехи в учёбе?»";
    }
}

std::vector<Choice> Semen::getChoices(const Player& player) const {
    std::vector<Choice> choices;
    choices.push_back({ "Спросить про учёбу", {}, "Семён даёт тебе несколько полезных советов." });
    choices.push_back({ "Попросить конспекты",
        {{ChoiceEffect::IncreaseIntellect, 8}, {ChoiceEffect::IncreaseSemenRelation, 3}},
        "Семён даёт тебе свои конспекты." });
    return choices;
}

// ---- Artem ----
Artem::Artem() : NPC("Артём", "Замкнутый программист, гений математики") {}

std::string Artem::getDialog(const Player& player) const {
    if (relationship < 30) {
        return "Артём не поднимает взгляда от ноутбука: «А, привет.»";
    } else {
        return "Артём: «О, " + player.getName() + "! Как тебе новый материал по матанализу?»";
    }
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
