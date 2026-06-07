#pragma once
#include <string>

class Menu {
public:
    static void ShowMainMenu();
    static void ShowGameOverMenu(const std::string& endingText);
    static void ShowControls();
    static int MainMenuChoice();
    static void ShowSettings();
};
