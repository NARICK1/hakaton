#include "ConsoleUI.h"
#include "../systems/FatigueSystem.h"
#include "../systems/HungerSystem.h"
#include "../systems/ReputationSystem.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif

void ConsoleUI::SetConsoleUTF8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Включаем поддержку Unicode в консоли Windows
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hConsole, &mode)) {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hConsole, mode);
    }
#endif
}

void ConsoleUI::ClearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleUI::PrintSeparator() {
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
}

void ConsoleUI::PrintHeader(const std::string& title) {
    PrintSeparator();
    // Центрируем заголовок
    std::string padded = "        " + title + "        ";
    std::cout << padded << "\n";
    PrintSeparator();
}

void ConsoleUI::PrintPlayerStats(const Player& player) {
    const auto& stats = player.getStats();
    std::cout << "\n=== Состояние " << player.getName() << " ===\n";
    std::cout << " Интеллект: " << stats.intellect << "/" << GameConstants::MAX_STAT << "\n";
    std::cout << " Энергия:   " << stats.energy << "/" << GameConstants::MAX_STAT << "\n";
    std::cout << " Усталость: " << stats.fatigue << "/" << GameConstants::MAX_FATIGUE << "\n";
    std::cout << " Голод:     " << stats.hunger << "/" << GameConstants::MAX_HUNGER << "\n";
    std::cout << " Стресс:    " << stats.stress << "/" << GameConstants::MAX_STAT << "\n";
    std::cout << " Человечность: " << stats.humanity << "/" << GameConstants::MAX_STAT << "\n";
    std::cout << " Здоровье:  " << stats.health << "/" << GameConstants::MAX_STAT << "\n";
    std::cout << " Деньги:    " << stats.money << " руб.\n";
    std::cout << " Романтика: " << stats.romance << "/" << GameConstants::MAX_STAT << "\n";
    std::cout << " Долгов:    " << player.getDebts() << "\n";
    std::cout << " Время:     " << player.getTimeString() << "\n";
    std::cout << " Локация:   " << locationToString(player.getLocation()) << "\n";

    // Статусы
    std::cout << "\n  Статусы:\n";
    std::cout << "  " << FatigueSystem::GetFatigueStatus(player) << "\n";
    std::cout << "  " << HungerSystem::GetHungerStatus(player) << "\n";
    std::cout << "  Репутация: " << ReputationSystem::GetReputationStatus(player) << "\n";

    // Активные баффы
    if (!player.getActiveBuffs().empty()) {
        std::cout << "\n  Активные эффекты:\n";
        for (auto b : player.getActiveBuffs()) {
            switch (b) {
            case BuffType::ImposterSyndrome: std::cout << "  ⚠ Синдром самозванца\n"; break;
            case BuffType::Burnout: std::cout << "  ⚠ Выгорание\n"; break;
            case BuffType::BrokenHeart: std::cout << "  ⚠ Разбитое сердце\n"; break;
            case BuffType::SleepParalysis: std::cout << "  ⚠ Сонный паралич\n"; break;
            case BuffType::Starvation: std::cout << "  ⚠ Голодание\n"; break;
            }
        }
    }

    // Оценки
    std::cout << "\n  Оценки:\n";
    const char* examNames[] = { "История", "ЯиМП", "Дискретка", "Матанализ", "Комп. сети" };
    for (int i = 0; i < 5; i++) {
        int g = player.getGrade(i + 1);
        if (g > 0) {
            std::cout << "  " << examNames[i] << ": " << g << "\n";
        } else {
            std::cout << "  " << examNames[i] << ": не сдан\n";
        }
    }
    PrintSeparator();
}

void ConsoleUI::PrintDayHeader(int day, const std::string& dayName) {
    ClearScreen();
    PrintHeader("ДЕНЬ " + std::to_string(day));
    std::cout << "\n               " << dayName << "\n";
    PrintSeparator();
}

void ConsoleUI::PrintStatus(const Player& player) {
    std::cout << "\n[" << player.getTimeString() << "] "
              << locationToString(player.getLocation())
              << " | Эн: " << player.getStats().energy
              << " Уст: " << player.getStats().fatigue
              << " Гол: " << player.getStats().hunger
              << " Стресс: " << player.getStats().stress
              << " Деньги: " << player.getStats().money << "\n";
}

void ConsoleUI::WaitForEnter() {
    std::cout << "\nНажмите Enter, чтобы продолжить...";
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

void ConsoleUI::PrintText(const std::string& text) {
    std::cout << text << "\n";
}

void ConsoleUI::PrintLocationMenu(LocationID location) {
    std::cout << "\nВы находитесь в локации: " << locationToString(location) << "\n";
    std::cout << "Доступные действия:\n";
}

int ConsoleUI::ShowMenu(const std::vector<std::string>& options) {
    for (size_t i = 0; i < options.size(); i++) {
        std::cout << (i + 1) << ". " << options[i] << "\n";
    }
    std::cout << "0. Назад/Выход\n";
    std::cout << "Ваш выбор: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');
    return choice;
}
