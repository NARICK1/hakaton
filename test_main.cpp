#include "core/Game.h"
#include "data/Lang.h"
#include "ui/ConsoleUI.h"
#include <iostream>

int main() {
    std::cout << "Start" << std::endl;
    Game game;
    std::cout << "Game created" << std::endl;
    std::cout << Lang::get("menu_new_game") << std::endl;
    std::cout << "Done" << std::endl;
    return 0;
}
