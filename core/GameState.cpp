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
    oss << "---NPC_MEMORY---\n";
    oss << npcMemoryData.size() << "\n";
    oss << npcMemoryData;
    oss << "\n---END---\n";
    return oss.str();
}

bool GameState::deserialize(const std::string& data) {
    std::istringstream iss(data);
    int phaseInt, reasonInt;
    std::string startedStr;

    if (!(iss >> phaseInt >> reasonInt >> currentDay >> startedStr)) return false;

    // Skip to PLAYER section
    std::string line;
    bool playerSectionFound = false;
    while (std::getline(iss, line)) {
        if (line == "---PLAYER---") {
            playerSectionFound = true;
            break;
        }
    }
    if (!playerSectionFound) return false;

    // Read all lines until ---NPC_MEMORY--- as player data
    std::string playerData;
    bool npcSectionFound = false;
    while (std::getline(iss, line)) {
        if (line == "---NPC_MEMORY---") {
            npcSectionFound = true;
            break;
        }
        if (!playerData.empty()) playerData += "\n";
        playerData += line;
    }

    currentPhase = static_cast<GamePhase>(phaseInt);
    gameOverReason = static_cast<GameOverCondition>(reasonInt);
    gameStarted = (startedStr == "1");

    if (playerData.empty()) return false;
    if (!player.deserialize(playerData)) return false;

    // Read NPC memory size and data
    npcMemoryData.clear();
    if (npcSectionFound) {
        size_t memSize;
        if (iss >> memSize) {
            iss.ignore(); // skip newline
            if (memSize > 0) {
                npcMemoryData.resize(memSize);
                iss.read(&npcMemoryData[0], memSize);
            }
        }
    }

    return true;
}
