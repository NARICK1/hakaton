#pragma once
#include "../player/Player.h"

class FatigueSystem {
public:
    static void Update(Player& player);
    static bool IsExhausted(const Player& player);
    static std::string GetFatigueStatus(const Player& player);
};
