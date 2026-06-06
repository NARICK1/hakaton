#pragma once
#include "Enums.h"
#include "Constants.h"
#include <string>
#include <vector>

struct GameConfig {
    std::string playerName = "Тимур";
    int totalDays = GameConstants::TOTAL_DAYS;
    bool enableRandomEvents = true;
    bool enableAutoSave = true;
};

struct ExamQuestion {
    std::string question;
    std::string correctAnswer;
    std::vector<std::string> options;
    bool isMultipleChoice = false;
};

struct Choice {
    std::string text;
    std::vector<std::pair<ChoiceEffect, int>> effects;
    std::string resultingText;
    int requiredDay = 0;
    int requiredRelation = 0;
    std::string requiredFlag;
};

struct DialogNode {
    std::string speaker;
    std::string text;
    std::vector<Choice> choices;
};

struct NPCMemoryEntry {
    std::string eventKey;
    int value = 0;
    bool triggered = false;
};
