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
    oss << "\n---JOURNAL---\n";
    oss << journalData.size() << "\n";
    oss << journalData;
    oss << "\n---ENCYCLOPEDIA---\n";
    oss << encyclopediaData.size() << "\n";
    oss << encyclopediaData;
    oss << "\n---ACHIEVEMENTS---\n";
    oss << achievementsData.size() << "\n";
    oss << achievementsData;
    oss << "\n---HABITS---\n";
    oss << habitsData.size() << "\n";
    oss << habitsData;
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
            iss.ignore();
            if (memSize > 0) {
                npcMemoryData.resize(memSize);
                iss.read(&npcMemoryData[0], memSize);
            }
        }
    }

    // Read remaining sections using helper
    auto readSection = [&](std::string& target, const std::string& marker) {
        std::string line;
        while (std::getline(iss, line)) {
            if (line == marker) {
                size_t sz;
                if (iss >> sz) {
                    iss.ignore();
                    if (sz > 0) {
                        target.resize(sz);
                        iss.read(&target[0], sz);
                    }
                }
                return true;
            }
        }
        return false;
    };

    readSection(journalData, "---JOURNAL---");
    readSection(encyclopediaData, "---ENCYCLOPEDIA---");
    readSection(achievementsData, "---ACHIEVEMENTS---");
    readSection(habitsData, "---HABITS---");

    return true;
}
