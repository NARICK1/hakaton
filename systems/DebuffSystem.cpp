#include "DebuffSystem.h"
#include "../data/Constants.h"
#include "../data/Lang.h"
#include <sstream>

void DebuffSystem::Update(Player& player) {
    auto& s = player.getStats();

    if (s.hunger >= GameConstants::STARVATION_THRESHOLD) {
        ApplyStarvation(player);
    } else {
        RemoveStarvation(player);
    }

    if (s.stress > 70 && s.intellect < 30) {
        ApplyImposterSyndrome(player);
    } else {
        RemoveImposterSyndrome(player);
    }

    if (s.fatigue >= GameConstants::FATIGUE_CRITICAL && s.energy < 30) {
        ApplyBurnout(player);
    } else if (s.fatigue < 50) {
        RemoveBurnout(player);
    }

    if (s.fatigue >= 95) {
        ApplySleepParalysis(player);
    } else if (s.fatigue < 60) {
        RemoveSleepParalysis(player);
    }

    player.applyBuffs();
}

void DebuffSystem::ApplyImposterSyndrome(Player& player) { player.addBuff(BuffType::ImposterSyndrome); }
void DebuffSystem::ApplyBurnout(Player& player) { player.addBuff(BuffType::Burnout); }
void DebuffSystem::ApplyBrokenHeart(Player& player) { player.addBuff(BuffType::BrokenHeart); }
void DebuffSystem::ApplySleepParalysis(Player& player) { player.addBuff(BuffType::SleepParalysis); }
void DebuffSystem::ApplyStarvation(Player& player) { player.addBuff(BuffType::Starvation); }

void DebuffSystem::RemoveImposterSyndrome(Player& player) { player.removeBuff(BuffType::ImposterSyndrome); }
void DebuffSystem::RemoveBurnout(Player& player) { player.removeBuff(BuffType::Burnout); }
void DebuffSystem::RemoveBrokenHeart(Player& player) { player.removeBuff(BuffType::BrokenHeart); }
void DebuffSystem::RemoveSleepParalysis(Player& player) { player.removeBuff(BuffType::SleepParalysis); }
void DebuffSystem::RemoveStarvation(Player& player) { player.removeBuff(BuffType::Starvation); }

std::string DebuffSystem::GetBuffDescription(const Player& player) {
    if (player.getActiveBuffs().empty()) return "";
    std::ostringstream oss;
    oss << " [";
    for (auto b : player.getActiveBuffs()) {
        switch (b) {
        case BuffType::ImposterSyndrome: oss << Lang::get("deb_imposter") << " "; break;
        case BuffType::Burnout: oss << Lang::get("deb_burnout") << " "; break;
        case BuffType::BrokenHeart: oss << Lang::get("deb_broken_heart") << " "; break;
        case BuffType::SleepParalysis: oss << Lang::get("deb_sleep") << " "; break;
        case BuffType::Starvation: oss << Lang::get("deb_starving") << " "; break;
        }
    }
    oss << "]";
    return oss.str();
}

bool DebuffSystem::HasAnyDebuff(const Player& player) {
    return !player.getActiveBuffs().empty();
}
