#include "EndingSystem.h"
#include "../data/Lang.h"
#include <iostream>
#include <sstream>

static std::string makeEndingFrame(const std::string& title, const std::string& body) {
    std::string result;
    int W = 72;
    std::string top = "+" + std::string(W, '=') + "+";
    std::string side = "|";
    result += top + "\n";

    // Title centered
    int lPad = (W - static_cast<int>(title.size())) / 2;
    int rPad = W - lPad - static_cast<int>(title.size());
    result += side + std::string(lPad, ' ') + title + std::string(rPad, ' ') + side + "\n";
    result += side + std::string(W, ' ') + side + "\n";

    // Body lines
    std::istringstream iss(body);
    std::string line;
    while (std::getline(iss, line)) {
        if (static_cast<int>(line.size()) > W) line = line.substr(0, W);
        result += side + " " + line + std::string(W - 1 - static_cast<int>(line.size()), ' ') + side + "\n";
    }

    result += side + std::string(W, ' ') + side + "\n";
    result += "+" + std::string(W, '=') + "+";
    return result;
}

GameOverCondition EndingSystem::CheckEnding(const Player& player) {
    if (player.getStats().health <= 0) {
        return GameOverCondition::StarvedToDeath;
    }
    const auto& stats = player.getStats();
    if (stats.hunger >= 100) {
        return GameOverCondition::StarvedToDeath;
    }
    if (stats.stress >= 100) {
        return GameOverCondition::MentalHospital;
    }
    return GameOverCondition::None;
}

GameOverCondition EndingSystem::EvaluateEnding(const Player& player) {
    const auto& stats = player.getStats();
    int avgGrade = 0;
    int gradeCount = 0;
    for (int i = 1; i <= 5; i++) {
        if (player.getGrade(i) > 0) {
            avgGrade += player.getGrade(i);
            gradeCount++;
        }
    }
    if (gradeCount > 0) avgGrade /= gradeCount;

    // Secret romantic ending
    if (player.hasFlag("romantic_ending") &&
        stats.romance >= 70 &&
        player.getRelation("Alla") >= 80 &&
        avgGrade >= 60) {
        return GameOverCondition::SecretRomantic;
    }

    // Secret army ending
    if (player.hasFlag("army_path")) {
        return GameOverCondition::SecretArmy;
    }

    // Eternal debtor
    if (player.getDebts() >= 4) {
        return GameOverCondition::EternalDebtor;
    }

    // Expelled
    if (player.getDebts() >= 2 && avgGrade < 50) {
        return GameOverCondition::Expelled;
    }

    // Academic leave
    if (stats.stress >= 80 && stats.fatigue >= 80) {
        return GameOverCondition::AcademicLeave;
    }

    // Score evaluation
    int totalScore = 0;
    totalScore += stats.intellect / 2;
    totalScore += stats.humanity;
    totalScore += stats.romance > 50 ? 20 : 0;
    totalScore += avgGrade;
    totalScore -= stats.stress;
    totalScore -= stats.fatigue / 2;
    totalScore += player.getRelation("Teachers");

    if (totalScore >= 300) return GameOverCondition::SuperEnding;
    if (totalScore >= 200) return GameOverCondition::GoodEnding;
    return GameOverCondition::NormalEnding;
}

static std::string endingTitle(GameOverCondition ending) {
    switch (ending) {
    case GameOverCondition::SuperEnding: return Lang::get("end_super");
    case GameOverCondition::GoodEnding: return Lang::get("end_good");
    case GameOverCondition::NormalEnding: return Lang::get("end_normal");
    case GameOverCondition::Expelled: return Lang::get("end_expelled");
    case GameOverCondition::AcademicLeave: return Lang::get("end_academic");
    case GameOverCondition::MentalHospital: return Lang::get("end_mental");
    case GameOverCondition::StarvedToDeath: return Lang::get("end_starved");
    case GameOverCondition::EternalDebtor: return Lang::get("end_debtor");
    case GameOverCondition::SecretRomantic: return Lang::get("end_romantic");
    case GameOverCondition::SecretArmy: return Lang::get("end_army");
    default: return Lang::get("end_unknown");
    }
}

static std::string endingBody(GameOverCondition ending) {
    switch (ending) {
    case GameOverCondition::SuperEnding: return Lang::get("end_super_body");
    case GameOverCondition::GoodEnding: return Lang::get("end_good_body");
    case GameOverCondition::NormalEnding: return Lang::get("end_normal_body");
    case GameOverCondition::Expelled: return Lang::get("end_expelled_body");
    case GameOverCondition::AcademicLeave: return Lang::get("end_academic_body");
    case GameOverCondition::MentalHospital: return Lang::get("end_mental_body");
    case GameOverCondition::StarvedToDeath: return Lang::get("end_starved_body");
    case GameOverCondition::EternalDebtor: return Lang::get("end_debtor_body");
    case GameOverCondition::SecretRomantic: return Lang::get("end_romantic_body");
    case GameOverCondition::SecretArmy: return Lang::get("end_army_body");
    default: return Lang::get("end_unknown");
    }
}

std::string EndingSystem::GetEndingText(GameOverCondition ending) {
    return makeEndingFrame(endingTitle(ending) + " (" + gameOverToString(ending) + ")", endingBody(ending));
}
