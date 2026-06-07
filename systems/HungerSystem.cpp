#include "HungerSystem.h"
#include "../data/Constants.h"
#include <algorithm>

void HungerSystem::Update(Player& player) {
    auto& stats = player.getStats();

    // hunger = сытость:
    // 100 = сыт
    // 0 = умирает от голода
    if (stats.hunger <= GameConstants::STARVATION_THRESHOLD) {
        stats.health -= 1;
        stats.energy -= 2;
        player.addBuff(BuffType::Starvation);
    } else {
        player.removeBuff(BuffType::Starvation);
    }

    stats.clampAll();
}

bool HungerSystem::IsStarving(const Player& player) {
    return player.getStats().hunger <= GameConstants::STARVATION_THRESHOLD;
}

std::string HungerSystem::GetHungerStatus(const Player& player) {
    int h = player.getStats().hunger;

    if (h >= 80) return "Вы сыты.";
    if (h >= 60) return "Вы почти сыты.";
    if (h >= 40) return "Вы хотите есть.";
    if (h >= 20) return "Вы голодны.";

    return "Вы умираете от голода!";
}

void HungerSystem::Eat(Player& player, int moneySpent) {
    auto& stats = player.getStats();

    if (stats.money >= moneySpent) {
        stats.money -= moneySpent;

        // Еда восстанавливает сытость, а не уменьшает её
        stats.hunger = std::min(
            GameConstants::MAX_HUNGER,
            stats.hunger + GameConstants::EAT_HUNGER_RESTORE
        );

        stats.energy = std::min(
            GameConstants::MAX_STAT,
            stats.energy + GameConstants::EAT_ENERGY_GAIN
        );

        player.removeBuff(BuffType::Starvation);
    }

    stats.clampAll();
}