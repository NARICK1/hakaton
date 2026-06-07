#include "core/Game.h"
#include "ui/ConsoleUI.h"
#include "ui/Menu.h"
#include "data/Lang.h"
#include "data/UIMode.h"
#include <iostream>

int main() {
    ConsoleUI::SetConsoleUTF8();
    if (!UIModeManager::isRussian()) {
        Lang::set(Lang::English);
    }
    Menu::ShowMainMenu();
    std::cout << "Menu shown" << std::endl;
    int ch = Menu::MainMenuChoice();
    std::cout << "Choice: " << ch << std::endl;
    if (ch == 3) Menu::ShowControls();
    if (ch == 4) Menu::ShowSettings();
    std::cout << "Done" << std::endl;
    return 0;
}
