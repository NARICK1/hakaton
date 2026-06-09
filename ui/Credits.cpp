#include "Credits.h"
#include "ConsoleUI.h"
#include "../data/UIMode.h"
#include "../player/Player.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace {
    int countLinesInFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return 0;
        }

        int lines = 0;
        std::string line;

        while (std::getline(file, line)) {
            lines++;
        }

        return lines;
    }

    int countProjectLines() {
        const std::vector<std::string> files = {
            "main.cpp",
            "core/Game.cpp",
            "core/GameState.cpp",
            "player/Player.cpp",
            "npc/NPC.cpp",
            "data/AsciiArt.cpp",
            "data/Lang.cpp",
            "exams/Exam.cpp",
            "events/RandomEvent.cpp",
            "systems/Achievements.cpp",
            "systems/DebuffSystem.cpp",
            "systems/Encyclopedia.cpp",
            "systems/EndingSystem.cpp",
            "systems/EventJournal.cpp",
            "systems/FatigueSystem.cpp",
            "systems/Habits.cpp",
            "systems/HungerSystem.cpp",
            "systems/RelationshipSystem.cpp",
            "systems/ReputationSystem.cpp",
            "ui/ConsoleUI.cpp",
            "ui/Credits.cpp",
            "ui/DevMode.cpp",
            "ui/Menu.cpp",
            "save/SaveManager.cpp"
        };

        int total = 0;

        for (const auto& file : files) {
            total += countLinesInFile(file);
        }

        return total;
    }

    void printBoxLine(const std::string& text) {
        int w = UIModeManager::screenW();
        std::cout << BOX_V << rpad("  " + text, w) << BOX_V << "\n";
    }
}

void Credits::ShowCreditsMain() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader("АВТОРЫ ПРОЕКТА", DECO_SPECIAL);
    showProjectInfo();
    showTeam();
    showFinalThanks();
    ConsoleUI::WaitForEnter();
}

void Credits::ShowEndingCredits(const Player& player, const std::string& endingName) {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader("ФИНАЛЬНЫЕ ТИТРЫ", DECO_SPECIAL);
    showPlayerStats(player, endingName);
    showProjectInfo();
    showTeam();
    showFinalThanks();
    ConsoleUI::WaitForEnter();
}

void Credits::showProjectInfo() {
    printBoxLine("Будни студента");
    printBoxLine("Жанр: консольная сюжетная игра / студенческий survival-management");
    printBoxLine("Движок: C++17, консольный интерфейс");
    printBoxLine("Строк кода в основных .cpp файлах: " + std::to_string(countProjectLines()));
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
}

void Credits::showTeam() {
    printBoxLine("Команда проекта:");
    printBoxLine("Дудин С.В. — программирование, сценарий, игровые системы");
    printBoxLine("Тимур — главный герой игры");
    printBoxLine("Алла, Булат, Семён, Артём — NPC и сюжетные ветки");
    printBoxLine("Преподаватели — экзамены, долги и давление университета");
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
}

void Credits::showPlayerStats(const Player& player, const std::string& endingName) {
    const auto& s = player.getStats();

    printBoxLine("Полученная концовка: " + endingName);
    printBoxLine("День: " + std::to_string(player.getCurrentDay()));
    printBoxLine("Деньги: " + std::to_string(s.money));
    printBoxLine("Интеллект: " + std::to_string(s.intellect));
    printBoxLine("Сытость: " + std::to_string(s.hunger));
    printBoxLine("Стресс: " + std::to_string(s.stress));
    printBoxLine("Романтика: " + std::to_string(s.romance));
    printBoxLine("Отношения с Аллой: " + std::to_string(player.getRelation("Алла")));
    printBoxLine("Долги: " + std::to_string(player.getDebts()));
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
}

void Credits::showFinalThanks() {
    printBoxLine("Спасибо за игру.");
    printBoxLine("Отдельный респект тем, кто не сломал проект через Git.");
    printBoxLine("Нажми Enter, чтобы вернуться назад.");
}
