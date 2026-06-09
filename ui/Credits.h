#pragma once
#include <string>

class Player;

class Credits {
public:
    static void ShowCreditsMain();
    static void ShowEndingCredits(const Player& player, const std::string& endingName);

private:
    static void showProjectInfo();
    static void showTeam();
    static void showPlayerStats(const Player& player, const std::string& endingName);
    static void showFinalThanks();
};
