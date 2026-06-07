#include "ui/ConsoleUI.h"
#include "ui/Menu.h"
#include "data/UIMode.h"
#include "data/Lang.h"
#include <iostream>
#include <windows.h>

int main() {
    __try {
        ConsoleUI::SetConsoleUTF8();
        if (!UIModeManager::isRussian()) Lang::set(Lang::English);
        std::cout << "Reached ShowMainMenu" << std::endl;
        Menu::ShowMainMenu();
        std::cout << "Menu done" << std::endl;
        int x;
        std::cin >> x;
        return 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        std::cout << "Exception: " << GetExceptionCode() << std::endl;
        return 1;
    }
}
