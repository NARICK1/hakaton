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

    // ---- Logo block: render outside the box frame ----
    // Variant A: Unicode (UUST) — always displayed
    const char* logoUnicode[] = {
        "██╗   ██╗██╗   ██╗███████╗████████╗",
        "██║   ██║██║   ██║██╔════╝╚══██╔══╝",
        "██║   ██║██║   ██║███████╗   ██║",
        "██║   ██║██║   ██║╚════██║   ██║",
        "╚██████╔╝╚██████╔╝███████║   ██║",
        " ╚═════╝  ╚═════╝ ╚══════╝   ╚═╝"
    };

    // Variant B: ASCII fallback (unused by default — swap logo pointer if needed)
    const char* logoAscii[] = {
        "##   ## ##   ## ######## ########",
        "##   ## ##   ## ##    ## ##   ##",
        "##   ## ##   ## ########    ##",
        "##   ## ##   ## ##          ##",
        "##   ## ##   ## ##    ##    ##",
        " #####   #####  ########    ##"
    };

    // Use Unicode variant by default
    const char** logo = logoUnicode;
    const int LOGO_LINES = 6;

    // Find max visual width among logo lines
    int logoMaxW = 0;
    for (int i = 0; i < LOGO_LINES; i++) {
        int v = static_cast<int>(visLen(logo[i]));
        if (v > logoMaxW) logoMaxW = v;
    }

    // Find max visual width of title
    int titleW = static_cast<int>(visLen(Lang::get("menu_title")));

    // Block width = max(logo width, title width)
    int blockW = (logoMaxW > titleW) ? logoMaxW : titleW;

    // One fixed left pad for the entire block
    int blockPad = (w - blockW) / 2;
    if (blockPad < 0) blockPad = 0;

    // Logo top spacing
    for (int i = 0; i < 2; i++) std::cout << "\n";

    // Print logo as a single block with fixed left padding
    for (int i = 0; i < LOGO_LINES; i++) {
        int lineW = static_cast<int>(visLen(logo[i]));
        int rightPad = blockW - lineW;
        if (rightPad < 0) rightPad = 0;
        std::cout << std::string(blockPad, ' ') << logo[i]
                  << std::string(rightPad, ' ');
        // Animate each line with a small delay
        sleepMs(15);
        std::cout << "\n";
    }
    sleepMs(80);

    // Title: same left pad as the logo block
    int titleLineW = static_cast<int>(visLen(Lang::get("menu_title")));
    int titleRightPad = blockW - titleLineW;
    if (titleRightPad < 0) titleRightPad = 0;
    std::cout << std::string(blockPad, ' ') << Lang::get("menu_title")
              << std::string(titleRightPad, ' ') << "\n";
    sleepMs(40);

    // Logo bottom spacing
    std::cout << "\n";

    // ---- Menu box frame (decorative) ----
    std::string d = DECO_EVENT;
    int dLen = static_cast<int>(visLen(d));
    int midW = w - dLen * 2;
    if (midW < 0) midW = 0;

    std::cout << d << std::string(midW, BOX_H[0]) << d << "\n";

    // Welcome messages
    std::cout << BOX_V " " << rpad(Lang::get("menu_welcome"), w - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad(Lang::get("menu_welcome2"), w - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad(Lang::get("menu_welcome3"), w - 2) << " " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    std::cout << d << std::string(midW, BOX_H[0]) << d << "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    // Menu items with animation
    std::string items[] = {
        Lang::get("menu_new_game"),
        Lang::get("menu_load_game"),
        Lang::get("menu_controls"),
        Lang::get("menu_settings"),
        Lang::get("menu_debug"),
        Lang::get("menu_credits"),
        Lang::get("menu_exit")
    };
    int nums[] = {1, 2, 3, 4, 5, 6, 0};
    for (int i = 0; i < 7; i++) {
        std::string line = "    [" + std::to_string(nums[i]) + "] " + items[i];
        std::cout << BOX_V " " << rpad(line, w - 2) << " " BOX_V "\n";
        sleepMs(50);
    }
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << d << std::string(midW, BOX_H[0]) << d << "\n";
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
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V " 1. " << Lang::get("menu_restart") << "\n";
    std::cout << BOX_V " 2. " << Lang::get("menu_back") << "\n";
    std::cout << BOX_V " 0. " << Lang::get("menu_exit") << "\n";
    int ww = W();
    std::string d = DECO_SPECIAL;
    int dl = static_cast<int>(visLen(d));
    int mw = ww - dl * 2;
    if (mw < 0) mw = 0;
    std::cout << d << std::string(mw, BOX_H[0]) << d << "\n";
}

void Menu::ShowControls() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader(Lang::get("ctrl_title"), DECO_EVENT);
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
    std::string d2 = DECO_EVENT;
    int dl2 = static_cast<int>(visLen(d2));
    int mw2 = w - dl2 * 2;
    if (mw2 < 0) mw2 = 0;
    std::cout << d2 << std::string(mw2, BOX_H[0]) << d2 << "\n";
    ConsoleUI::WaitForEnter();
}

void Menu::ShowSettings() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader(Lang::get("set_title"), DECO_EVENT);
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
    std::string d3 = DECO_EVENT;
    int dl3 = static_cast<int>(visLen(d3));
    int mw3 = w - dl3 * 2;
    if (mw3 < 0) mw3 = 0;
    std::cout << d3 << std::string(mw3, BOX_H[0]) << d3 << "\n";
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
