#include <iostream>
#include <string>
#include "ui/ConsoleUI.h"
#include "data/UIMode.h"

// Minimal reproductions from ShowMainMenu/Menu.cpp
int main() {
    ConsoleUI::SetConsoleUTF8();
    int w = UIModeManager::screenW();
    
    std::cout << "+" << std::string(w, '=') << "+" << std::endl;
    
    auto asciiLine = [&](const std::string& text) {
        size_t vlen = 0;
        for (size_t i = 0; i < text.size(); i++) {
            unsigned char c = static_cast<unsigned char>(text[i]);
            if (c < 0x80) vlen++;
            else if (c < 0xC0) continue;
            else vlen++;
        }
        int l = (w - static_cast<int>(vlen)) / 2;
        if (l < 0) l = 0;
        std::cout << "|" << std::string(l, ' ') << text
                  << std::string(w - l - static_cast<int>(text.size()), ' ') << "|" << std::endl;
    };

    asciiLine("  _   _ _   _ _   _ ___ _____");
    std::cout << "OK 1" << std::endl;
    asciiLine(" | | | | | | | \\ | |_ _|_   _|");
    std::cout << "OK 2" << std::endl;
    asciiLine(" | | | | | | |  \\| || |  | |");
    std::cout << "OK 3" << std::endl;
    asciiLine(" | |_| | |_| | |\\  || |  | |");
    std::cout << "OK 4" << std::endl;
    asciiLine("  \\___/ \\___/ |_| \\_|___| |_|");
    std::cout << "DONE" << std::endl;
    return 0;
}
