#pragma once
#include "../player/Player.h"

class ReputationSystem {
public:
    static void UpdateReputation(Player& player);
    static std::string GetReputationStatus(const Player& player);
};
