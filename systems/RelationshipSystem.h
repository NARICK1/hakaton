#pragma once
#include "../player/Player.h"
#include <string>
#include <vector>
#include <utility>

class RelationshipSystem {
public:
    static void ApplyChoiceEffect(Player& player, const std::string& npcName,
        const std::vector<std::pair<ChoiceEffect, int>>& effects);
    static std::string GetRelationshipLevelText(int relationship);
    static std::string GetNPCDialogBasedOnRelation(const std::string& npcName, int relationship);
};
