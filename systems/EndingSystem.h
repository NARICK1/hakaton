#pragma once
#include "../player/Player.h"
#include "../data/Enums.h"

class EndingSystem {
public:
    static GameOverCondition CheckEnding(const Player& player);
    static std::string GetEndingText(GameOverCondition ending);
    static GameOverCondition EvaluateEnding(const Player& player);
};
