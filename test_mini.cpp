#include <iostream>
#include <string>
#include "ui/ConsoleUI.h"
#include "data/Lang.h"
#include "data/UIMode.h"

int main() {
    ConsoleUI::SetConsoleUTF8();
    if (!UIModeManager::isRussian()) Lang::set(Lang::English);

    int w = UIModeManager::screenW();
    std::cout << "+" << std::string(w, '=') << "+" << std::endl;
    std::cout << "|" << std::string(w, ' ') << "|" << std::endl;
    std::cout << "|" "  _   _ _   _ _   _ ___ _____" << "  |" << std::endl;
    std::cout << "|" << std::string(w, ' ') << "|" << std::endl;
    std::cout << "+" << std::string(w, '=') << "+" << std::endl;
    std::cout << "Test with Russian: " << Lang::get("menu_new_game") << std::endl;
    std::cout << "All done, enter a number: ";
    int x;
    std::cin >> x;
    return 0;
}
