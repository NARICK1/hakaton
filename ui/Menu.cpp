#include "Menu.h"
#include "ConsoleUI.h"
#include "../data/UIMode.h"
#include "../data/Lang.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static int W() { return UIModeManager::screenW(); }

static void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ---- Safe Animated Main Menu ----

void Menu::ShowMainMenu() {
    ConsoleUI::ClearScreen();
    int w = W();

    auto asciiLine = [&](const std::string& text) {
        size_t vlen = visLen(text);
        int l = (w - static_cast<int>(vlen)) / 2;
        if (l < 0) l = 0;
        int pad = w - l - static_cast<int>(vlen);
        if (pad < 0) pad = 0;
        std::cout << BOX_V << std::string(l, ' ') << text
                  << std::string(pad, ' ') << BOX_V "\n";
    };

    auto emptyLine = [&]() {
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    };

    // Top border
    std::cout << BOX_TL << std::string(w, BOX_H[0]) << BOX_TR "\n";
    emptyLine();

    // Animated ASCII logo lines
    const char* logo[] = {
        "  _   _ _   _ _   _ ___ _____",
        " | | | | | | | \\ | |_ _|_   _|",
        " | | | | | | |  \\| || |  | |",
        " | |_| | |_| | |\\  || |  | |",
        "  \\___/ \\___/ |_| \\_|___| |_|"
    };
    for (const auto& line : logo) {
        ConsoleUI::AnimateLogoLine(line, w, 10);
    }
    sleepMs(40);
    emptyLine();

    // Title lines
    ConsoleUI::AnimateLogoLine("UUST / UUNI T", w, 5);
    sleepMs(30);
    ConsoleUI::AnimateLogoLine(Lang::get("menu_title"), w, 5);
    sleepMs(40);
    emptyLine();

    std::cout << BOX_L << std::string(w, BOX_H[0]) << BOX_R "\n";
    emptyLine();

    // Welcome messages
    std::cout << BOX_V " " << rpad(Lang::get("menu_welcome"), w - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad(Lang::get("menu_welcome2"), w - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad(Lang::get("menu_welcome3"), w - 2) << " " BOX_V "\n";
    emptyLine();

    std::cout << BOX_L << std::string(w, BOX_H[0]) << BOX_R "\n";
    emptyLine();

    // Menu items with animation
    std::vector<std::string> menuLines;
    std::string items[] = {
        Lang::get("menu_new_game"),
        Lang::get("menu_load_game"),
        Lang::get("menu_controls"),
        Lang::get("menu_settings"),
        Lang::get("menu_exit")
    };
    int nums[] = {1, 2, 3, 4, 0};
    for (int i = 0; i < 5; i++) {
        std::string line = "    [" + std::to_string(nums[i]) + "] " + items[i];
        std::cout << BOX_V " " << rpad(line, w - 2) << " " BOX_V "\n";
        sleepMs(50);
    }
    emptyLine();
    std::cout << BOX_BL << std::string(w, BOX_H[0]) << BOX_BR "\n";
}

int Menu::MainMenuChoice() {
    std::cout << " " << Lang::get("ui_your_choice") << ": " << std::flush;
    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');
    if (choice == 0) return 0;
    return choice;
}

void Menu::ShowGameOverMenu(const std::string& endingText) {
    ConsoleUI::ClearScreen();
    std::cout << endingText << "\n";
    ConsoleUI::PrintSeparator();
    std::cout << BOX_V " 1. " << Lang::get("menu_restart") << "\n";
    std::cout << BOX_V " 2. " << Lang::get("menu_back") << "\n";
    std::cout << BOX_V " 0. " << Lang::get("menu_exit") << "\n";
    std::cout << BOX_BL << std::string(W(), BOX_H[0]) << BOX_BR "\n";
}

void Menu::ShowControls() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader(Lang::get("ctrl_title"));
    int w = W();
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("ctrl_text1"), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("ctrl_text2"), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("ctrl_tips"), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_tip1"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_tip2"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_tip3"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_tip4"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_tip5"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_tip6"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("ctrl_endings"), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_end1"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_end2"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V "  - " << rpad(Lang::get("ctrl_end3"), w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_BL << std::string(w, BOX_H[0]) << BOX_BR "\n";
    ConsoleUI::WaitForEnter();
}

void Menu::ShowSettings() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader(Lang::get("set_title"));
    int w = W();
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("menu_profile") + ": "
        + UIModeManager::currentName(), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("menu_lang_current") + ": "
        + Lang::currentName(), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("menu_profile") + ":", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "    1. " << rpad(Lang::get("menu_compact"), w - 10) << "  " BOX_V "\n";
    std::cout << BOX_V "    2. " << rpad(Lang::get("menu_standard"), w - 10) << "  " BOX_V "\n";
    std::cout << BOX_V "    3. " << rpad(Lang::get("menu_wide"), w - 10) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("menu_lang") + ":", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "    4. " << rpad(Lang::get("menu_rus"), w - 10) << "  " BOX_V "\n";
    std::cout << BOX_V "    5. " << rpad(Lang::get("menu_eng"), w - 10) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("0. " + Lang::get("menu_back_title"), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_BL << std::string(w, BOX_H[0]) << BOX_BR "\n";
    std::cout << " " << Lang::get("ui_your_choice") << ": " << std::flush;
    int ch;
    std::cin >> ch;
    std::cin.ignore(10000, '\n');
    if (ch == 1) { UIModeManager::setCompact(); ConsoleUI::WaitForEnter(); }
    else if (ch == 2) { UIModeManager::setStandard(); ConsoleUI::WaitForEnter(); }
    else if (ch == 3) { UIModeManager::setWide(); ConsoleUI::WaitForEnter(); }
    else if (ch == 4) { Lang::set(Lang::Russian); UIModeManager::setLang(0); ConsoleUI::WaitForEnter(); }
    else if (ch == 5) { Lang::set(Lang::English); UIModeManager::setLang(1); ConsoleUI::WaitForEnter(); }
}
