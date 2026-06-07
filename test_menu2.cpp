#include "ui/Menu.h"
#include "ui/ConsoleUI.h"
#include "data/UIMode.h"
#include "data/Lang.h"
#include <iostream>

int main() {
    ConsoleUI::SetConsoleUTF8();
    if (!UIModeManager::isRussian()) Lang::set(Lang::English);
    Menu::ShowMainMenu();
    std::cout << "Menu done" << std::endl;
    int x;
    std::cin >> x;
    return 0;
}
