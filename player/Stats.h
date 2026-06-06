#pragma once
#include "../data/Constants.h"
#include <algorithm>

struct Stats {
    int intellect = GameConstants::START_INTELLECT;
    int energy = GameConstants::START_ENERGY;
    int fatigue = GameConstants::START_FATIGUE;
    int hunger = GameConstants::START_HUNGER;
    int stress = GameConstants::START_STRESS;
    int humanity = GameConstants::START_HUMANITY;
    int money = GameConstants::START_MONEY;
    int romance = GameConstants::START_ROMANCE;
    int health = 100;

    void clampAll() {
        intellect = std::clamp(intellect, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
        energy = std::clamp(energy, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
        fatigue = std::clamp(fatigue, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
        hunger = std::clamp(hunger, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
        stress = std::clamp(stress, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
        humanity = std::clamp(humanity, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
        money = std::clamp(money, 0, GameConstants::MAX_MONEY);
        romance = std::clamp(romance, GameConstants::MIN_STAT, GameConstants::MAX_STAT);
        health = std::clamp(health, 0, GameConstants::MAX_STAT);
    }

    bool isAlive() const { return health > 0 && hunger < GameConstants::MAX_HUNGER; }
};
