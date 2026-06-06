#include "SaveManager.h"
#include "../player/Player.h"
#include <fstream>
#include <iostream>

const std::string SaveManager::SAVE_FILE = "savegame.dat";

bool SaveManager::SaveGame(const GameState& state) {
    std::ofstream file(SAVE_FILE, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл для сохранения.\n";
        return false;
    }

    std::string data = state.serialize();
    size_t size = data.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    file.write(data.c_str(), size);

    file.close();
    std::cout << "Игра сохранена.\n";
    return true;
}

bool SaveManager::LoadGame(GameState& state) {
    std::ifstream file(SAVE_FILE, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Сохранение не найдено.\n";
        return false;
    }

    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    std::string data(size, '\0');
    file.read(&data[0], size);

    file.close();

    if (state.deserialize(data)) {
        std::cout << "Игра загружена.\n";
        return true;
    }

    std::cerr << "Ошибка загрузки сохранения.\n";
    return false;
}

bool SaveManager::SaveExists() {
    std::ifstream file(SAVE_FILE);
    return file.good();
}

bool SaveManager::DeleteSave() {
    return std::remove(SAVE_FILE.c_str()) == 0;
}
