#pragma once
#include "../core/GameState.h"
#include <string>

class SaveManager {
private:
    static const std::string SAVE_FILE;

public:
    static bool SaveGame(const GameState& state);
    static bool LoadGame(GameState& state);
    static bool SaveExists();
    static bool DeleteSave();
};
