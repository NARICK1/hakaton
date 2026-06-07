#include "ReputationSystem.h"
#include <algorithm>
#include <vector>

void ReputationSystem::UpdateReputation(Player& player) {
    player.getStats().clampAll();
}

std::string ReputationSystem::GetReputationStatus(const Player& player) {
    int avgRelation = 0;
    int count = 0;

    const std::vector<std::string> npcs = {
        "Алла",
        "Булат",
        "Семён",
        "Артём",
        "Преподаватели"
    };

    for (const auto& npc : npcs) {
        avgRelation += player.getRelation(npc);
        count++;
    }

    if (count > 0) {
        avgRelation /= count;
    }

    int humanity = player.getStats().humanity;
    int stress = player.getStats().stress;
    int debts = player.getDebts();

    int reputationScore = avgRelation;
    reputationScore += (humanity - 50) / 4;
    reputationScore -= stress / 10;
    reputationScore -= debts * 5;
    reputationScore = std::clamp(reputationScore, 0, 100);

    if (reputationScore >= 85) {
        return "Всеобщий любимец";
    }

    if (reputationScore >= 65) {
        return "Уважаемый человек";
    }

    if (reputationScore >= 45) {
        return "Средняя репутация";
    }

    if (reputationScore >= 25) {
        return "К вам относятся настороженно";
    }

    return "Изгой";
}
