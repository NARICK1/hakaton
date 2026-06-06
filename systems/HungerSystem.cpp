#include "HungerSystem.h"

void HungerSystem::Update(Player& player) {
    auto& stats = player.getStats();
    if (stats.hunger >= GameConstants::STARVATION_THRESHOLD) {
        stats.health -= 1;
        stats.energy -= 2;
        player.addBuff(BuffType::Starvation);
    } else {
        player.removeBuff(BuffType::Starvation);
    }
    stats.clampAll();
}

bool HungerSystem::IsStarving(const Player& player) {
    return player.getStats().hunger >= GameConstants::STARVATION_THRESHOLD;
}

std::string HungerSystem::GetHungerStatus(const Player& player) {
    int h = player.getStats().hunger;
    if (h >= 80) return "Вы умираете от голода!";
    if (h >= 60) return "Вы очень голодны.";
    if (h >= 40) return "Вы хотите есть.";
    if (h >= 20) return "Вы слегка голодны.";
    return "Вы сыты.";
}

void HungerSystem::Eat(Player& player, int moneySpent) {
    auto& stats = player.getStats();
    if (stats.money >= moneySpent) {
        stats.money -= moneySpent;
        stats.hunger = std::max(0, stats.hunger - GameConstants::EAT_HUNGER_REDUCE);
        stats.energy = std::min(GameConstants::MAX_STAT, stats.energy + GameConstants::EAT_ENERGY_GAIN);
        player.removeBuff(BuffType::Starvation);
    }
    stats.clampAll();
}
