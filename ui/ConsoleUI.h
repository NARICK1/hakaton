#pragma once
#include "../player/Player.h"
#include <string>
#include <vector>

class ConsoleUI {
public:
    static void ClearScreen();
    static void PrintSeparator();
    static void PrintHeader(const std::string& title);
    static void PrintPlayerStats(const Player& player);
    static void PrintLocationMenu(LocationID location);
    static void PrintDayHeader(int day, const std::string& dayName);
    static void PrintStatus(const Player& player);
    static void WaitForEnter();
    static void PrintText(const std::string& text);
    static void SetConsoleUTF8();
    static int ShowMenu(const std::vector<std::string>& options);
};
