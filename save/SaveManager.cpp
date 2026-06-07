#include "SaveManager.h"
#include "../player/Player.h"
#include "../data/Lang.h"
#include <fstream>
#include <iostream>

const std::string SaveManager::SAVE_FILE = "savegame.dat";

bool SaveManager::SaveGame(const GameState& state) {
    std::ofstream file(SAVE_FILE, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << Lang::get("save_error_open") << "\n";
        return false;
    }

    std::string data = state.serialize();
    size_t size = data.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    file.write(data.c_str(), size);

    file.close();
    std::cout << Lang::get("ui_saved") << "\n";
    return true;
}

bool SaveManager::LoadGame(GameState& state) {
    std::ifstream file(SAVE_FILE, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << Lang::get("ui_save_not_found") << "\n";
        return false;
    }

    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    std::string data(size, '\0');
    file.read(&data[0], size);

    file.close();

    if (state.deserialize(data)) {
        std::cout << Lang::get("ui_loaded") << "\n";
        return true;
    }

    std::cerr << Lang::get("save_error_load") << "\n";
    return false;
}

bool SaveManager::SaveExists() {
    std::ifstream file(SAVE_FILE);
    return file.good();
}

bool SaveManager::DeleteSave() {
    return std::remove(SAVE_FILE.c_str()) == 0;
}
