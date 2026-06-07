#include "ui/ConsoleUI.h"
#include "ui/Menu.h"
#include "data/UIMode.h"
#include "data/Lang.h"
#include <iostream>

int main() {
    std::cout << "1" << std::endl;
    ConsoleUI::SetConsoleUTF8();
    std::cout << "2" << std::endl;
    if (!UIModeManager::isRussian()) Lang::set(Lang::English);
    std::cout << "3" << std::endl;
    // Just the menu choice, no ShowMainMenu
    std::cout << "  [1] New Game" << std::endl;
    std::cout << "  [0] Exit" << std::endl;
    std::cout << "Your choice: ";
    int x;
    std::cin >> x;
    return 0;
}
