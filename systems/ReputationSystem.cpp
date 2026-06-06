#include "ReputationSystem.h"
#include <vector>

void ReputationSystem::UpdateReputation(Player& /*player*/) {
    // Обновляется при событиях
}

std::string ReputationSystem::GetReputationStatus(const Player& player) {
    int avgRelation = 0;
    int count = 0;

    std::vector<std::string> npcs = { "Алла", "Булат", "Семён", "Артём", "Преподаватели" };
    for (const auto& npc : npcs) {
        avgRelation += player.getRelation(npc);
        count++;
    }

    if (count > 0) avgRelation /= count;

    if (avgRelation >= 80) return "Всеобщий любимец";
    if (avgRelation >= 60) return "Уважаемый человек";
    if (avgRelation >= 40) return "Средняя репутация";
    if (avgRelation >= 20) return "К вам относятся настороженно";
    return "Изгой";
}
