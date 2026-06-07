#include "Credits.h"
#include "ConsoleUI.h"
#include "../data/UIMode.h"
#include "../data/Lang.h"
#include "../player/Player.h"
#include "../core/GameState.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

static int W() { return UIModeManager::screenW(); }
static void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ========== Подсчёт строк кода ==========

static int countLinesInFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return 0;
    int n = 0;
    std::string l;
    while (std::getline(f, l)) n++;
    return n;
}

static int countSourceLines() {
    const char* files[] = {
        "main.cpp",
        "core/Game.cpp", "core/GameState.cpp",
        "player/Player.cpp",
        "npc/NPC.cpp",
        "data/AsciiArt.cpp", "data/Lang.cpp",
        "exams/Exam.cpp",
        "events/RandomEvent.cpp",
        "systems/RelationshipSystem.cpp", "systems/FatigueSystem.cpp",
        "systems/HungerSystem.cpp", "systems/ReputationSystem.cpp",
        "systems/EndingSystem.cpp", "systems/DebuffSystem.cpp",
        "systems/EventJournal.cpp", "systems/Encyclopedia.cpp",
        "systems/Achievements.cpp", "systems/Habits.cpp",
        "ui/ConsoleUI.cpp", "ui/Menu.cpp", "ui/DevMode.cpp", "ui/Credits.cpp",
        "save/SaveManager.cpp"
    };
    int total = 0;
    for (auto& f : files) {
        total += countLinesInFile(f);
    }
    return total;
}

static int countHeaderLines() {
    const char* files[] = {
        "core/Game.h", "core/GameState.h",
        "player/Player.h", "player/Stats.h",
        "npc/NPC.h",
        "data/AsciiArt.h", "data/Lang.h", "data/UIMode.h",
        "data/Enums.h", "data/Constants.h", "data/GameConfig.h",
        "exams/Exam.h",
        "events/RandomEvent.h",
        "systems/RelationshipSystem.h", "systems/FatigueSystem.h",
        "systems/HungerSystem.h", "systems/ReputationSystem.h",
        "systems/EndingSystem.h", "systems/DebuffSystem.h",
        "systems/EventJournal.h", "systems/Encyclopedia.h",
        "systems/Achievements.h", "systems/Habits.h",
        "ui/ConsoleUI.h", "ui/Menu.h", "ui/DevMode.h", "ui/Credits.h",
        "save/SaveManager.h"
    };
    int total = 0;
    for (auto& f : files) {
        total += countLinesInFile(f);
    }
    return total;
}

static int countProjectFiles() {
    const char* files[] = {
        "main.cpp",
        "core/Game.cpp", "core/GameState.cpp", "core/Game.h", "core/GameState.h",
        "player/Player.cpp", "player/Player.h", "player/Stats.h",
        "npc/NPC.cpp", "npc/NPC.h",
        "data/AsciiArt.cpp", "data/AsciiArt.h", "data/Lang.cpp", "data/Lang.h",
        "data/UIMode.h", "data/Enums.h", "data/Constants.h", "data/GameConfig.h",
        "exams/Exam.cpp", "exams/Exam.h",
        "events/RandomEvent.cpp", "events/RandomEvent.h",
        "systems/RelationshipSystem.cpp", "systems/RelationshipSystem.h",
        "systems/FatigueSystem.cpp", "systems/FatigueSystem.h",
        "systems/HungerSystem.cpp", "systems/HungerSystem.h",
        "systems/ReputationSystem.cpp", "systems/ReputationSystem.h",
        "systems/EndingSystem.cpp", "systems/EndingSystem.h",
        "systems/DebuffSystem.cpp", "systems/DebuffSystem.h",
        "systems/EventJournal.cpp", "systems/EventJournal.h",
        "systems/Encyclopedia.cpp", "systems/Encyclopedia.h",
        "systems/Achievements.cpp", "systems/Achievements.h",
        "systems/Habits.cpp", "systems/Habits.h",
        "ui/ConsoleUI.cpp", "ui/ConsoleUI.h", "ui/Menu.cpp", "ui/Menu.h",
        "ui/DevMode.cpp", "ui/DevMode.h", "ui/Credits.cpp", "ui/Credits.h",
        "save/SaveManager.cpp", "save/SaveManager.h",
        "build.bat", "build.ps1"
    };
    return sizeof(files) / sizeof(files[0]);
}

// ========== Анимации участников ==========

static void memberGlitch(const std::string& name, const std::string& role) {
    int w = W();
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << rpad("  " + Lang::get("credits_title"), w) << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // медленный glitch
    for (int g = 0; g < 6; g++) {
        std::cout << BOX_V "\r";
        for (int j = 0; j < w - 2; j++) {
            if (rand() % 4 == 0) std::cout << char(rand() % 94 + 33);
            else std::cout << " ";
        }
        std::cout << " " BOX_V "\r" << std::flush;
        sleepMs(120);
    }
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\r" << std::flush;
    sleepMs(150);
    // имя появляется посимвольно
    std::cout << BOX_V "  " << std::flush;
    for (size_t i = 0; i < name.size(); i++) {
        std::cout << name[i] << std::flush;
        sleepMs(60);
    }
    std::cout << "\r" << BOX_V << rpad("  " + name, w) << BOX_V "\n";
    sleepMs(200);
    // роль — цифровые помехи
    for (int g = 0; g < 4; g++) {
        std::cout << BOX_V "\r" << "  " << std::flush;
        for (size_t i = 0; i < role.size(); i++) {
            if (rand() % 5 == 0) std::cout << char(rand() % 94 + 33);
            else std::cout << role[i];
        }
        sleepMs(80);
    }
    std::cout << "\r" << BOX_V << rpad("  " + role, w) << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // эффект загрузки системы
    std::cout << BOX_V "  [";
    for (int i = 0; i < 20; i++) {
        std::cout << "#" << std::flush;
        sleepMs(50);
    }
    std::cout << "] " << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    sleepMs(200);
    ConsoleUI::WaitForEnter();
}

static void memberFlowers(const std::string& name, const std::string& role) {
    int w = W();
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << rpad("  " + Lang::get("credits_title"), w) << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // цветочки, сердечки, звёздочки сверху вниз
    const char* deco[] = { "*", "+", "x", "o", "." };
    for (int row = 0; row < 4; row++) {
        std::cout << BOX_V "  " << std::flush;
        for (int j = 0; j < w - 6; j++) {
            if (rand() % 4 == 0) std::cout << deco[rand() % 5];
            else std::cout << " ";
        }
        std::cout << "  " BOX_V "\r" << std::flush;
        sleepMs(200);
    }
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\r" << std::flush;
    sleepMs(150);
    // имя появляется с эффектом
    for (size_t i = 0; i < name.size(); i++) {
        std::cout << "\r" << BOX_V << rpad("  " + name.substr(0, i + 1), w) << std::flush;
        sleepMs(80);
    }
    std::cout << "\n";
    sleepMs(300);
    std::cout << BOX_V << rpad("  " + role, w) << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // финальная строка декораций
    std::cout << BOX_V "  ";
    for (int i = 0; i < w - 6; i++) {
        if (i % 7 == 0) std::cout << "+";
        else if (i % 7 == 3) std::cout << "x";
        else std::cout << " ";
    }
    std::cout << "  " BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    sleepMs(200);
    ConsoleUI::WaitForEnter();
}

static void memberTech(const std::string& name, const std::string& role) {
    int w = W();
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << rpad("  " + Lang::get("credits_title"), w) << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // эффект сборки — строки сверху вниз
    const char* buildLines[] = {
        "[  ] 0%",
        "[##        ] 25%",
        "[#####     ] 50%",
        "[########  ] 75%",
        "[##########] 100%"
    };
    for (auto& bl : buildLines) {
        std::cout << BOX_V << rpad("  " + std::string(bl), w) << BOX_V "\r" << std::flush;
        sleepMs(250);
    }
    std::cout << "\n";
    sleepMs(200);
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\r" << std::flush;
    sleepMs(100);
    // имя посимвольно
    for (size_t i = 0; i < name.size(); i++) {
        std::cout << "\r" << BOX_V << rpad("  " + name.substr(0, i + 1), w) << std::flush;
        sleepMs(70);
    }
    std::cout << "\n";
    sleepMs(200);
    std::cout << BOX_V << rpad("  " + role, w) << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    sleepMs(200);
    ConsoleUI::WaitForEnter();
}

static void memberDynamic(const std::string& name, const std::string& role) {
    int w = W();
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << rpad("  " + Lang::get("credits_title"), w) << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // движение стрелок > > >
    for (int g = 0; g < 5; g++) {
        std::string arr;
        for (int k = 0; k <= g; k++) arr += ">";
        arr += " " + name;
        std::cout << BOX_V << rpad("  " + arr, w) << BOX_V "\r" << std::flush;
        sleepMs(180);
    }
    sleepMs(300);
    std::cout << "\n";
    std::cout << BOX_V << rpad("  " + role, w) << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // финальная строка
    std::cout << BOX_V "  ";
    for (int i = 0; i < w - 6; i++) {
        if (i % 5 == 0) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "  " BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    sleepMs(200);
    ConsoleUI::WaitForEnter();
}

static void memberPeaceful(const std::string& name, const std::string& role) {
    int w = W();
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << rpad("  " + Lang::get("credits_title"), w) << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // спокойное мерцание звёзд
    for (int g = 0; g < 5; g++) {
        std::cout << BOX_V "\r";
        std::cout << "  ";
        for (int j = 0; j < w - 6; j++) {
            if (rand() % 8 == 0) std::cout << ".";
            else std::cout << " ";
        }
        std::cout << "  " BOX_V "\r" << std::flush;
        sleepMs(200);
    }
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\r" << std::flush;
    sleepMs(150);
    // плавный fade имени
    for (int fade = 0; fade < 3; fade++) {
        std::cout << BOX_V << rpad("  " + name, w) << BOX_V "\r" << std::flush;
        sleepMs(100);
        std::cout << BOX_V << rpad("  " + std::string(visLen(name), ' '), w) << BOX_V "\r" << std::flush;
        sleepMs(100);
    }
    sleepMs(200);
    std::cout << BOX_V << rpad("  " + name, w) << BOX_V "\n";
    sleepMs(300);
    std::cout << BOX_V << rpad("  " + role, w) << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    // финальное мерцание
    for (int g = 0; g < 3; g++) {
        std::cout << BOX_V "  ";
        for (int j = 0; j < w - 6; j++) {
            if (rand() % 12 == 0) std::cout << ".";
            else std::cout << " ";
        }
        std::cout << "  " BOX_V "\r" << std::flush;
        sleepMs(180);
    }
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    sleepMs(200);
    ConsoleUI::WaitForEnter();
}

// ---- Team member display ----

void Credits::showTeamMember(const std::string& name, const std::string& role, const std::string& animationStyle) {
    if (animationStyle == "glitch") memberGlitch(name, role);
    else if (animationStyle == "flowers") memberFlowers(name, role);
    else if (animationStyle == "tech") memberTech(name, role);
    else if (animationStyle == "dynamic") memberDynamic(name, role);
    else memberPeaceful(name, role);
}

void Credits::waitEnter() {
    std::cout << "\n " << Lang::get("ui_press_enter") << " " << std::flush;
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

// ---- Statistics screen ----

void Credits::showStats(const Player& player, const std::string& endingName) {
    ConsoleUI::ClearScreen();
    int w = W();
    int srcLines = countSourceLines();
    int hdrLines = countHeaderLines();
    int totalLines = srcLines + hdrLines;
    int fileCount = countProjectFiles();
    ConsoleUI::PrintHeader(Lang::get("credits_stats"));

    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);

    auto ps = [&](const std::string& label, const std::string& val) {
        std::cout << BOX_V "  " << rpad(label + ": " + val, w - 4) << "  " BOX_V "\n";
    };

    ps(Lang::get("credits_days"), std::to_string(player.getCurrentDay()));
    int passedExams = 0;
    for (int i = 1; i <= 5; i++) {
        if (player.getGrade(i) >= 40) passedExams++;
    }
    ps(Lang::get("credits_exams"), std::to_string(passedExams) + " / 5");
    ps(Lang::get("credits_stress"), std::to_string(player.getStats().stress));
    ps(Lang::get("credits_money"), std::to_string(player.getStats().money));
    ps(Lang::get("credits_alla"), std::to_string(player.getRelation("Alla")));
    ps(Lang::get("credits_ending"), endingName);

    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    ps(Lang::get("credits_files"), std::to_string(fileCount));
    ps(Lang::get("credits_lines"), std::to_string(totalLines));
    ps(Lang::get("credits_modules"), "9 (core/npc/player/events/exams/systems/save/ui/data)");
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    waitEnter();
}

// ---- Final thanks ----

void Credits::showFinalThanks() {
    ConsoleUI::ClearScreen();
    int w = W();
    int w2 = (w - 44) / 2;
    if (w2 < 0) w2 = 0;
    sleepMs(500);

    // Верхняя граница
    std::cout << "\n\n";
    std::cout << "  " << std::string(w2, ' ') << "◆══════════════════════════════════════════◆\n";
    sleepMs(300);
    std::cout << "  " << std::string(w2, ' ') << "|                                            |\n";
    sleepMs(150);
    std::cout << "  " << std::string(w2, ' ') << "|       " << Lang::get("credits_thanks") << "       |\n";
    sleepMs(250);
    std::cout << "  " << std::string(w2, ' ') << "|                                            |\n";
    sleepMs(150);
    std::cout << "  " << std::string(w2, ' ') << "|         " << Lang::get("credits_game_title") << "          |\n";
    sleepMs(250);
    std::cout << "  " << std::string(w2, ' ') << "|                                            |\n";
    sleepMs(150);
    std::cout << "  " << std::string(w2, ' ') << "|              УУНИТ — 2026                |\n";
    sleepMs(250);
    std::cout << "  " << std::string(w2, ' ') << "|                                            |\n";
    sleepMs(150);
    std::cout << "  " << std::string(w2, ' ') << "◆══════════════════════════════════════════◆\n";
    sleepMs(500);
    ConsoleUI::WaitForEnter();
}

// ---- Main credits (from menu) ----

void Credits::ShowCreditsMain() {
    showTeamMember("Karpov Danil",   "Architecture, UI, animations, effects", "glitch");
    showTeamMember("Dudin Sergey",   "Game mechanics, system balance, logic", "tech");
    showTeamMember("Karimov Bulat",  "Game mechanics, days 5-8, events", "dynamic");
    showTeamMember("Joshbaeva Alia", "Script, days 1-4, dialogues, events", "flowers");
    showTeamMember("Mukanaev Timur", "Testing, team support, qa", "peaceful");

    ConsoleUI::ClearScreen();
    int w = W();
    int srcLines = countSourceLines();
    int hdrLines = countHeaderLines();
    int totalLines = srcLines + hdrLines;
    int fileCount = countProjectFiles();
    ConsoleUI::PrintHeader(Lang::get("credits_team_title"));
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    std::cout << BOX_V "  " << rpad(Lang::get("credits_team_size") + ": 5", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_lang") + ": C++17 / STL", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_libs") + ": " + Lang::get("credits_none"), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_gfx") + ": ASCII / Unicode", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_l10n") + ": Russian / English", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_profiles") + ": 3", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_year") + ": 2026", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_files") + ": " + std::to_string(fileCount), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad(Lang::get("credits_lines") + ": " + std::to_string(totalLines), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_SPECIAL);
    waitEnter();

    showFinalThanks();
}

// ---- Ending credits (called after game over) ----

void Credits::ShowEndingCredits(const Player& player, const std::string& endingName) {
    showStats(player, endingName);
    showTeamMember("Karpov Danil",   "Architecture, UI, animations, effects", "glitch");
    showTeamMember("Dudin Sergey",   "Game mechanics, system balance, logic", "tech");
    showTeamMember("Karimov Bulat",  "Game mechanics, days 5-8, events", "dynamic");
    showTeamMember("Joshbaeva Alia", "Script, days 1-4, dialogues, events", "flowers");
    showTeamMember("Mukanaev Timur", "Testing, team support, qa", "peaceful");
    showFinalThanks();
}

