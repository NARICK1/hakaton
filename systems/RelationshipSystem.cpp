#include "RelationshipSystem.h"

void RelationshipSystem::ApplyChoiceEffect(Player& player,
    const std::string& npcName,
    const std::vector<std::pair<ChoiceEffect, int>>& effects) {
    (void)npcName;

    auto& stats = player.getStats();

    for (const auto& [effect, value] : effects) {
        switch (effect) {
        case ChoiceEffect::IncreaseIntellect:
            stats.intellect += player.scaleGain(value);
            break;

        case ChoiceEffect::DecreaseIntellect:
            stats.intellect -= player.scalePenalty(value);
            break;

        case ChoiceEffect::IncreaseEnergy:
            stats.energy += player.scaleGain(value);
            break;

        case ChoiceEffect::DecreaseEnergy:
            stats.energy -= player.scalePenalty(value);
            break;

        case ChoiceEffect::IncreaseStress:
            stats.stress += player.scalePenalty(value);
            break;

        case ChoiceEffect::DecreaseStress:
            stats.stress -= player.scaleGain(value);
            break;

        case ChoiceEffect::IncreaseMoney:
            stats.money += player.scaleMoneyGain(value);
            break;

        case ChoiceEffect::DecreaseMoney:
            stats.money -= player.scaleCost(value);
            break;

        case ChoiceEffect::IncreaseRomance:
            stats.romance += player.scaleGain(value);
            break;

        case ChoiceEffect::DecreaseRomance:
            stats.romance -= player.scalePenalty(value);
            break;

        case ChoiceEffect::IncreaseAllaRelation:
            player.modifyRelation("Алла", value);
            break;

        case ChoiceEffect::DecreaseAllaRelation:
            player.modifyRelation("Алла", -value);
            break;

        case ChoiceEffect::IncreaseBulatRelation:
            player.modifyRelation("Булат", value);
            break;

        case ChoiceEffect::DecreaseBulatRelation:
            player.modifyRelation("Булат", -value);
            break;

        case ChoiceEffect::IncreaseSemenRelation:
            player.modifyRelation("Семён", value);
            break;

        case ChoiceEffect::DecreaseSemenRelation:
            player.modifyRelation("Семён", -value);
            break;

        case ChoiceEffect::IncreaseTeacherRelation:
            player.modifyRelation("Преподаватели", value);
            break;

        case ChoiceEffect::DecreaseTeacherRelation:
            player.modifyRelation("Преподаватели", -value);
            break;

        case ChoiceEffect::IncreaseHumanity:
            stats.humanity += player.scaleGain(value);
            break;

        case ChoiceEffect::DecreaseHumanity:
            stats.humanity -= player.scalePenalty(value);
            break;

        // Теперь hunger = сытость.
        // IncreaseHunger = повысить сытость.
        // DecreaseHunger = снизить сытость.
        case ChoiceEffect::IncreaseHunger:
            stats.hunger += player.scaleGain(value);
            break;

        case ChoiceEffect::DecreaseHunger:
            stats.hunger -= player.scalePenalty(value);
            break;

        case ChoiceEffect::None:
            break;
        }
    }

    stats.clampAll();
}

std::string RelationshipSystem::GetRelationshipLevelText(int relationship) {
    if (relationship >= 80) {
        return "отличные";
    }

    if (relationship >= 60) {
        return "хорошие";
    }

    if (relationship >= 40) {
        return "нейтральные";
    }

    if (relationship >= 20) {
        return "плохие";
    }

    return "враждебные";
}

std::string RelationshipSystem::GetNPCDialogBasedOnRelation(const std::string& npcName, int relationship) {
    std::string prefix = npcName + ": ";

    if (relationship >= 80) {
        return prefix + "«Рад(а) тебя видеть!»";
    }

    if (relationship >= 60) {
        return prefix + "«Привет! Как дела?»";
    }

    if (relationship >= 40) {
        return prefix + "«Здравствуй.»";
    }

    if (relationship >= 20) {
        return prefix + "«Чего тебе?»";
    }

    return prefix + "«...»";
}
