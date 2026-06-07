#include "ui/ConsoleUI.h"
#include "data/UIMode.h"
#include "data/Lang.h"
#include <iostream>

int main() {
    ConsoleUI::SetConsoleUTF8();
    if (!UIModeManager::isRussian()) Lang::set(Lang::English);
    // Just print something
    std::cout << "Hello" << std::endl;
    std::cout << "Press enter: ";
    int x;
    std::cin >> x;
    return 0;
}
