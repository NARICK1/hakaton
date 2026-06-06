#include "GameState.h"
#include <sstream>

std::string GameState::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(currentPhase) << "\n";
    oss << static_cast<int>(gameOverReason) << "\n";
    oss << currentDay << "\n";
    oss << (gameStarted ? "1" : "0") << "\n";
    oss << "---PLAYER---\n";
    oss << player.serialize();
    return oss.str();
}

bool GameState::deserialize(const std::string& data) {
    std::istringstream iss(data);
    int phaseInt, reasonInt;
    std::string startedStr, playerSection, playerData;

    if (!(iss >> phaseInt >> reasonInt >> currentDay >> startedStr)) return false;

    // Пропускаем разделитель и читаем данные игрока
    std::string line;
    bool playerSectionFound = false;
    while (std::getline(iss, line)) {
        if (line == "---PLAYER---") {
            playerSectionFound = true;
            break;
        }
    }

    if (!playerSectionFound) return false;

    // Читаем все оставшиеся строки как данные игрока
    std::string remaining;
    std::getline(iss, remaining, '\0');
    if (!remaining.empty() && remaining.back() == '\0') {
        remaining.pop_back();
    }

    currentPhase = static_cast<GamePhase>(phaseInt);
    gameOverReason = static_cast<GameOverCondition>(reasonInt);
    gameStarted = (startedStr == "1");

    if (remaining.empty()) return false;
    return player.deserialize(remaining);
}
