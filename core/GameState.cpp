#include "GameState.h"
#include <sstream>

namespace {
    void writeBlock(std::ostringstream& oss, const std::string& marker, const std::string& data) {
        oss << marker << "\n";
        oss << data.size() << "\n";
        oss << data;
        oss << "\n";
    }

    bool readSizedBlock(std::istringstream& iss, std::string& out) {
        size_t size = 0;

        if (!(iss >> size)) {
            return false;
        }

        iss.ignore();
        out.clear();

        if (size > 0) {
            out.resize(size);
            iss.read(&out[0], size);
        }

        return true;
    }
}

std::string GameState::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(currentPhase) << "\n";
    oss << static_cast<int>(gameOverReason) << "\n";
    oss << currentDay << "\n";
    oss << (gameStarted ? "1" : "0") << "\n";

    oss << "---PLAYER---\n";
    oss << player.serialize();

    writeBlock(oss, "---NPC_MEMORY---", npcMemoryData);
    writeBlock(oss, "---JOURNAL---", journalData);
    writeBlock(oss, "---ENCYCLOPEDIA---", encyclopediaData);
    writeBlock(oss, "---ACHIEVEMENTS---", achievementsData);
    writeBlock(oss, "---HABITS---", habitsData);

    oss << "---END---\n";
    return oss.str();
}

bool GameState::deserialize(const std::string& data) {
    std::istringstream iss(data);
    int phaseInt = 0;
    int reasonInt = 0;
    std::string startedStr;

    if (!(iss >> phaseInt >> reasonInt >> currentDay >> startedStr)) {
        return false;
    }

    std::string line;
    bool playerSectionFound = false;

    while (std::getline(iss, line)) {
        if (line == "---PLAYER---") {
            playerSectionFound = true;
            break;
        }
    }

    if (!playerSectionFound) {
        return false;
    }

    std::string playerData;
    bool npcSectionFound = false;

    while (std::getline(iss, line)) {
        if (line == "---NPC_MEMORY---") {
            npcSectionFound = true;
            break;
        }

        if (!playerData.empty()) {
            playerData += "\n";
        }

        playerData += line;
    }

    currentPhase = static_cast<GamePhase>(phaseInt);
    gameOverReason = static_cast<GameOverCondition>(reasonInt);
    gameStarted = (startedStr == "1");

    if (playerData.empty()) {
        return false;
    }

    if (!player.deserialize(playerData)) {
        return false;
    }

    npcMemoryData.clear();
    journalData.clear();
    encyclopediaData.clear();
    achievementsData.clear();
    habitsData.clear();

    if (npcSectionFound) {
        if (!readSizedBlock(iss, npcMemoryData)) {
            npcMemoryData.clear();
        }
    }

    while (std::getline(iss, line)) {
        if (line.empty()) {
            continue;
        }

        if (line == "---END---") {
            break;
        }

        if (line == "---JOURNAL---") {
            readSizedBlock(iss, journalData);
        } else if (line == "---ENCYCLOPEDIA---") {
            readSizedBlock(iss, encyclopediaData);
        } else if (line == "---ACHIEVEMENTS---") {
            readSizedBlock(iss, achievementsData);
        } else if (line == "---HABITS---") {
            readSizedBlock(iss, habitsData);
        }
    }

    return true;
}
