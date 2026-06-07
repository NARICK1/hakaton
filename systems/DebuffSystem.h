#pragma once
#include "../player/Player.h"

class DebuffSystem {
public:
    // Check conditions and apply/remove debuffs
    static void Update(Player& player);

    // Manual triggers
    static void ApplyImposterSyndrome(Player& player);
    static void ApplyBurnout(Player& player);
    static void ApplyBrokenHeart(Player& player);
    static void ApplySleepParalysis(Player& player);
    static void ApplyStarvation(Player& player);

    // Removal
    static void RemoveImposterSyndrome(Player& player);
    static void RemoveBurnout(Player& player);
    static void RemoveBrokenHeart(Player& player);
    static void RemoveSleepParalysis(Player& player);
    static void RemoveStarvation(Player& player);

    // Status strings
    static std::string GetBuffDescription(const Player& player);
    static bool HasAnyDebuff(const Player& player);
};
