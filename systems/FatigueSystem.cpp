#include "FatigueSystem.h"

void FatigueSystem::Update(Player& player) {
    auto& stats = player.getStats();
    if (stats.fatigue >= GameConstants::FATIGUE_CRITICAL) {
        stats.energy -= 2;
        stats.stress += 1;
    }
    if (player.hasBuff(BuffType::SleepParalysis)) {
        stats.fatigue += 1;
    }
    stats.clampAll();
}

bool FatigueSystem::IsExhausted(const Player& player) {
    return player.getStats().fatigue >= GameConstants::FATIGUE_CRITICAL;
}

std::string FatigueSystem::GetFatigueStatus(const Player& player) {
    int f = player.getStats().fatigue;
    if (f >= 80) return "Крайняя степень усталости!";
    if (f >= 60) return "Сильная усталость.";
    if (f >= 40) return "Умеренная усталость.";
    if (f >= 20) return "Лёгкая усталость.";
    return "Вы полны сил.";
}
