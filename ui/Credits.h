#pragma once
#include <string>

class Player;
class GameState;

class Credits {
public:
    // Полные титры (из главного меню)
    static void ShowCreditsMain();

    // Титры после концовки (с краткой статистикой)
    static void ShowEndingCredits(const Player& player, const std::string& endingName);

private:
    static void showTeamMember(const std::string& name, const std::string& role, const std::string& animationStyle);
    static void showStats(const Player& player, const std::string& endingName);
    static void showFinalThanks();
    static void waitEnter();
};
