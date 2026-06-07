#pragma once

#include "../player/Player.h"
#include <string>

class HungerSystem {
public:
    static void Update(Player& player);
    static bool IsStarving(const Player& player);
    static std::string GetHungerStatus(const Player& player);
    static void Eat(Player& player, int moneySpent);
};