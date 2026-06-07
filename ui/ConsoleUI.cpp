#include "ConsoleUI.h"
#include "../data/AsciiArt.h"
#include "../data/Lang.h"
#include "../data/UIMode.h"
#include "../systems/FatigueSystem.h"
#include "../systems/HungerSystem.h"
#include "../systems/ReputationSystem.h"
#include "../systems/DebuffSystem.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// ---- UTF-8 helpers ----

size_t visLen(const std::string& s) {
    size_t len = 0;
    for (size_t i = 0; i < s.size(); i++) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) len++;
        else if (c < 0xC0) continue;
        else len++;
    }
    return len;
}

std::string rpad(const std::string& s, int visWidth) {
    size_t vlen = visLen(s);
    if (vlen >= static_cast<size_t>(visWidth))
        return s.substr(0, static_cast<size_t>(visWidth));
    return s + std::string(visWidth - vlen, ' ');
}

static std::string lpad(const std::string& s, int visWidth) {
    size_t vlen = visLen(s);
    if (vlen >= static_cast<size_t>(visWidth))
        return s.substr(0, static_cast<size_t>(visWidth));
    return std::string(visWidth - vlen, ' ') + s;
}

std::vector<std::string> wordWrap(const std::string& text, int visWidth) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word;
    std::string currentLine;
    while (stream >> word) {
        size_t wLen = visLen(word);
        size_t cLen = visLen(currentLine);
        if (cLen + wLen + (cLen > 0 ? 1 : 0) > static_cast<size_t>(visWidth)) {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word;
            } else {
                lines.push_back(word);
            }
        } else {
            if (!currentLine.empty()) currentLine += " ";
            currentLine += word;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);
    return lines;
}

static std::string lc(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ---- ConsoleUI implementation ----

void ConsoleUI::SetConsoleUTF8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void ConsoleUI::ClearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleUI::WaitForEnter() {
    int totalW = UIModeManager::screenW();
    std::cout << "\n"
              << BOX_V " " << Lang::get("ui_press_enter") << "\n"
              << BOX_BL << std::string(totalW, BOX_H[0]) << BOX_BR "\n";
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

int ConsoleUI::ShowMenu(const std::vector<std::string>& options,
                         const std::string& prompt) {
    int totalW = UIModeManager::screenW();
    int maxW = std::min(60, totalW);
    for (const auto& o : options) {
        int olen = static_cast<int>(visLen(o) + 4);
        if (olen > maxW) maxW = olen;
    }
    maxW = std::min(maxW, totalW);

    std::cout << BOX_TL << std::string(maxW, BOX_H[0]) << BOX_TR "\n";
    for (size_t i = 0; i < options.size(); i++) {
        std::string line = std::to_string(i + 1) + ". " + options[i];
        std::cout << BOX_V " " << rpad(line, maxW - 2) << BOX_V "\n";
    }
    std::cout << BOX_L << std::string(maxW, BOX_H[0]) << BOX_R "\n";
    std::string p = " " + prompt + ": ";
    std::cout << BOX_V << rpad(p, maxW) << BOX_V "\n";
    std::cout << BOX_BL << std::string(maxW, BOX_H[0]) << BOX_BR "\n";
    std::cout << "> ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');
    return choice;
}

// ---- Status Bars ----

std::string ConsoleUI::MakeBarString(int value, int maxVal, int width) {
    if (maxVal <= 0 || width <= 0) return std::string(width, BAR_EMPTY[0]);
    float ratio = std::clamp(static_cast<float>(value) / maxVal, 0.0f, 1.0f);
    int filled = static_cast<int>(ratio * width);
    int half = 0;
    if (filled < width) {
        float remainder = (ratio * width) - filled;
        if (remainder > 0.3f) half = 1;
    }
    std::string r;
    r += std::string(filled, BAR_FULL[0]);
    if (half) r += BAR_HALF[0];
    r += std::string(width - filled - half, BAR_EMPTY[0]);
    return r;
}

std::string ConsoleUI::MakeTopBar(const Player& player) {
    int totalW = UIModeManager::screenW();
    std::string dayStr = Lang::get("hud_day") + ": "
                       + std::to_string(player.getCurrentDay());
    std::string timeStr = player.getTimeString();
    std::string locStr = locationToString(player.getLocation());
    return rpad(dayStr + " | " + timeStr + " | " + locStr, totalW);
}

std::string ConsoleUI::MakeStatBar(const Player& player) {
    const auto& s = player.getStats();
    int totalW = UIModeManager::screenW();

    // Labels: ИНТ ЭН УСТ ГОЛ СТР РУБ (for Russian) / INT EN FAT HUN STR RUB (for English)
    std::string il = Lang::get("hud_int");
    std::string el = Lang::get("hud_en");
    std::string fl = Lang::get("hud_fat");
    std::string hl = Lang::get("hud_hun");
    std::string sl = Lang::get("hud_str");
    std::string ml = Lang::get("hud_money");

    std::ostringstream ss;
    ss << il << ":" << s.intellect
       << " " << el << ":" << s.energy
       << " " << fl << ":" << s.fatigue
       << " " << hl << ":" << s.hunger
       << " " << sl << ":" << s.stress
       << " " << ml << ":" << s.money;
    std::string text = ss.str();

    // All bars same length (12)
    std::string bars = " ["
        + MakeBarString(s.intellect, GameConstants::MAX_STAT, 12) + "]"
        + "[" + MakeBarString(s.energy, GameConstants::MAX_STAT, 12) + "]"
        + "[" + MakeBarString(100 - s.fatigue, 100, 12) + "]"
        + "[" + MakeBarString(100 - s.hunger, 100, 12) + "]"
        + "[" + MakeBarString(100 - s.stress, 100, 12) + "]"
        + "[" + MakeBarString(s.money, 5000, 12) + "]";
    return text + "  " + bars;
}

// ---- Unified Screen Renderer ----

void ConsoleUI::RenderScreen(const std::string& sceneTitle,
                              const std::string& mainText,
                              const std::vector<std::string>& choices,
                              const Player& player,
                              const std::string& rightPanel,
                              const std::string& sceneNPC,
                              const std::string& sceneLocation,
                              bool isExam,
                              bool isRain,
                              bool isEnding,
                              bool endingSuccess) {
    ClearScreen();

    int totalW = UIModeManager::screenW();
    int mainW = UIModeManager::mainW();
    int rpW = UIModeManager::rpW();

    if (totalW <= 0) totalW = 78;
    if (mainW <= 0) mainW = totalW - 4 - 22;
    if (rpW <= 0) rpW = totalW - 4 - mainW;

    // Build right panel content
    std::string rpContent = rightPanel.empty()
        ? AutoRightPanel(player, sceneNPC, sceneLocation, isExam, isRain, isEnding, endingSuccess)
        : rightPanel;

    std::vector<std::string> rpLines;
    std::istringstream rpStream(rpContent);
    std::string line;
    while (std::getline(rpStream, line)) {
        rpLines.push_back(line);
    }

    // Wrap story text
    std::vector<std::string> storyLines = wordWrap(mainText, mainW);
    if (storyLines.empty()) storyLines.push_back("");

    // Determine rows from profile
    int storyRows = std::min(UIModeManager::storyRows(), static_cast<int>(storyLines.size()));
    int remainingLines = static_cast<int>(storyLines.size()) - storyRows;
    int rpHeight = std::max(storyRows, 4);

    // ---- TOP FRAME ----
    std::cout << BOX_TL << std::string(totalW, BOX_H[0]) << BOX_TR "\n";

    // ---- HUD SECTION ----
    // Scene title / day name centered
    std::string title = " " + sceneTitle + " ";
    int tLen = static_cast<int>(visLen(title));
    int lPad = (totalW - tLen) / 2;
    std::cout << BOX_V << rpad(std::string(lPad, ' ') + title, totalW) << BOX_V "\n";
    std::cout << BOX_L << std::string(totalW, BOX_H[0]) << BOX_R "\n";

    // HUD Line 1: День | Время | Локация
    std::string hud1 = MakeTopBar(player);
    std::cout << BOX_V << rpad(" " + hud1, totalW) << BOX_V "\n";

    // HUD Line 2: Статы + бары
    std::string hud2 = MakeStatBar(player);
    std::cout << BOX_V << rpad(" " + hud2, totalW) << BOX_V "\n";

    // Split separator
    std::cout << BOX_L << std::string(totalW, BOX_H[0]) << BOX_R "\n";

    // ---- MAIN CONTENT: Story + Right Panel ----
    int maxContentRows = std::max(rpHeight, static_cast<int>(rpLines.size()));
    maxContentRows = std::max(maxContentRows, 6);

    for (int i = 0; i < maxContentRows; i++) {
        std::string leftPart = i < static_cast<int>(storyLines.size())
            ? storyLines[i] : "";
        leftPart = rpad(leftPart, mainW);

        std::string rightPart = i < static_cast<int>(rpLines.size())
            ? rpLines[i] : "";
        rightPart = rpad(rightPart, rpW);

        std::cout << BOX_V " " << leftPart << " " BOX_V " " << rightPart << BOX_V "\n";
    }

    // Dialog separator
    std::cout << BOX_L << std::string(totalW, BOX_H[0]) << BOX_R "\n";

    // ---- DIALOG SECTION ----
    if (remainingLines > 0) {
        // NPC name in dialog (extract from last story line if starts with special marker)
        std::string dialogHeader;
        int startIdx = static_cast<int>(storyLines.size()) - remainingLines;

        // If sceneNPC is set, show NPC name as dialog header
        if (!sceneNPC.empty()) {
            dialogHeader = sceneNPC;
            std::cout << BOX_V << rpad(" " + dialogHeader, totalW) << BOX_V "\n";
            std::cout << BOX_L << std::string(totalW, BOX_H[0]) << BOX_R "\n";
        }

        int dialogLines = std::min(remainingLines, 3);
        for (int i = 0; i < dialogLines; i++) {
            std::string dl = storyLines[startIdx + i];
            std::cout << BOX_V << rpad(" " + dl, totalW) << BOX_V "\n";
        }
    } else {
        std::cout << BOX_V << rpad("", totalW) << BOX_V "\n";
    }

    // Choices separator
    std::cout << BOX_L << std::string(totalW, BOX_H[0]) << BOX_R "\n";

    // ---- CHOICES SECTION ----
    for (size_t i = 0; i < choices.size(); i++) {
        std::string ch = std::to_string(i + 1) + ". " + choices[i];
        if (visLen(ch) > static_cast<size_t>(totalW))
            ch = ch.substr(0, static_cast<size_t>(totalW));
        std::cout << BOX_V << rpad(" " + ch, totalW) << BOX_V "\n";
    }

    // Bottom frame + prompt
    std::string prompt = " " + Lang::get("ui_your_choice") + " [1-"
                       + std::to_string(choices.size()) + "]: ";
    std::cout << BOX_BL << std::string(totalW, BOX_H[0]) << BOX_BR "\n";
    std::cout << BOX_V << rpad(prompt, totalW) << BOX_V "\n";
    std::cout << BOX_BL << std::string(totalW, BOX_H[0]) << BOX_BR "\n";
    std::cout << "> ";
}

// ---- Auto Right Panel ----

std::string ConsoleUI::AutoRightPanel(const Player& player,
                                       const std::string& npcName,
                                       const std::string& locationName,
                                       bool isExam, bool isRain,
                                       bool isEnding, bool endingSuccess) {
    std::string art;
    int rpW = UIModeManager::rpW();

    if (isEnding) {
        art = endingSuccess ? AsciiArt::GetSuccessEnding() : AsciiArt::GetFailEnding();
    } else if (isExam) {
        art = AsciiArt::ForExam();
    } else if (!npcName.empty()) {
        art = AsciiArt::ForNPC(npcName);
    } else if (isRain) {
        art = AsciiArt::GetRain();
    } else if (!locationName.empty()) {
        std::string ln = lc(locationName);
        if (ln.find("home") != std::string::npos || ln.find("дом") != std::string::npos)
            art = AsciiArt::GetHome();
        else if (ln.find("univers") != std::string::npos || ln.find("универ") != std::string::npos)
            art = AsciiArt::GetUniversity();
        else if (ln.find("auditor") != std::string::npos || ln.find("аудитор") != std::string::npos || ln.find("class") != std::string::npos)
            art = AsciiArt::GetClassroom();
        else if (ln.find("cant") != std::string::npos || ln.find("стол") != std::string::npos || ln.find("cafe") != std::string::npos)
            art = AsciiArt::GetCafeteria();
        else if (ln.find("street") != std::string::npos || ln.find("улиц") != std::string::npos)
            art = AsciiArt::GetStreet();
        else
            art = AsciiArt::ForLocation(static_cast<int>(player.getLocation()));
    } else {
        art = AsciiArt::ForLocation(static_cast<int>(player.getLocation()));
    }

    // Build panel
    std::string result;
    std::istringstream artStream(art);
    std::string aLine;
    while (std::getline(artStream, aLine)) {
        result += rpad(aLine, rpW) + "\n";
    }

    // Separator line
    result += std::string(rpW, '-') + "\n";

    // Stats
    const auto& stats = player.getStats();
    result += " " + Lang::get("hud_en") + ":" + MakeBarString(stats.energy, GameConstants::MAX_STAT, 10) + "\n";
    result += " " + Lang::get("hud_fat") + ":" + MakeBarString(100 - stats.fatigue, 100, 10) + "\n";
    result += " " + Lang::get("hud_str") + ":" + MakeBarString(100 - stats.stress, 100, 10) + "\n";
    result += " " + Lang::get("hud_hun") + ":" + MakeBarString(100 - stats.hunger, 100, 10) + "\n";

    // Debuffs
    if (DebuffSystem::HasAnyDebuff(player)) {
        result += " " + DebuffSystem::GetBuffDescription(player) + "\n";
    }

    return result;
}

// ---- Legacy wrappers ----

void ConsoleUI::PrintSeparator() {
    std::cout << BOX_L << std::string(UIModeManager::screenW(), BOX_H[0]) << BOX_R "\n";
}

void ConsoleUI::PrintHeader(const std::string& title) {
    int totalW = UIModeManager::screenW();
    std::string t = " " + title + " ";
    int tPad = (totalW - static_cast<int>(visLen(t))) / 2;
    std::cout << BOX_TL << std::string(totalW, BOX_H[0]) << BOX_TR "\n";
    std::cout << BOX_V << rpad(std::string(tPad, ' ') + t, totalW) << BOX_V "\n";
    std::cout << BOX_BL << std::string(totalW, BOX_H[0]) << BOX_BR "\n";
}

void ConsoleUI::PrintPlayerStats(const Player& player) {
    const auto& stats = player.getStats();
    int totalW = UIModeManager::screenW();

    std::cout << BOX_TL << std::string(totalW, BOX_H[0]) << BOX_TR "\n";
    std::cout << BOX_V " " << rpad(
        player.getName() + " | " + player.getTimeString()
        + " | " + Lang::get("hud_day") + " " + std::to_string(player.getCurrentDay())
        + " | " + locationToString(player.getLocation()),
        totalW - 2) << " " BOX_V "\n";
    PrintSeparator();

    auto printStat = [&](const std::string& name, int val, int mx) {
        std::cout << BOX_V " " << rpad(
            name + ": " + std::to_string(val) + "/" + std::to_string(mx)
            + "  " + MakeBarString(val, mx, 30), totalW - 2)
                  << " " BOX_V "\n";
    };

    printStat(Lang::get("hud_int"), stats.intellect, GameConstants::MAX_STAT);
    printStat(Lang::get("hud_en"), stats.energy, GameConstants::MAX_STAT);
    printStat(Lang::get("hud_fat"), stats.fatigue, GameConstants::MAX_FATIGUE);
    printStat(Lang::get("hud_hun"), stats.hunger, GameConstants::MAX_HUNGER);
    printStat(Lang::get("hud_str"), stats.stress, GameConstants::MAX_STAT);
    std::cout << BOX_V " " << rpad(Lang::get("hud_money") + ": "
        + std::to_string(stats.money), totalW - 2) << " " BOX_V "\n";

    std::cout << BOX_V " " << rpad("Долги: " + std::to_string(player.getDebts()), totalW - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad("Репутация: " + ReputationSystem::GetReputationStatus(player), totalW - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad("Усталость: " + FatigueSystem::GetFatigueStatus(player), totalW - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad("Голод: " + HungerSystem::GetHungerStatus(player), totalW - 2) << " " BOX_V "\n";

    if (!player.getActiveBuffs().empty()) {
        std::cout << BOX_V " " << rpad("Эффекты: "
            + DebuffSystem::GetBuffDescription(player), totalW - 2) << " " BOX_V "\n";
    }

    std::cout << BOX_V " Оценки: ";
    const char* examNames[] = { "История", "ЯиМП", "Дискретка", "Матанализ", "Комп.сети" };
    bool first = true;
    for (int i = 0; i < 5; i++) {
        int g = player.getGrade(i + 1);
        if (!first) std::cout << ", ";
        if (g > 0) std::cout << examNames[i] << ": " << g;
        else std::cout << examNames[i] << ": --";
        first = false;
    }
    std::cout << "\n";
    std::cout << BOX_BL << std::string(totalW, BOX_H[0]) << BOX_BR "\n";
}

void ConsoleUI::ShowDayTransition(int day, const std::string& dayName) {
    ClearScreen();
    int totalW = UIModeManager::screenW();
    std::cout << BOX_TL << std::string(totalW, BOX_H[0]) << BOX_TR "\n";
    for (int i = 0; i < totalW; i++) {
        std::cout << "\r" BOX_V << std::string(i, '=') << ">" << std::flush;
        sleepMs(5);
    }
    std::cout << "\n";
    sleepMs(200);

    std::string title = " " + Lang::get("hud_day") + " " + std::to_string(day) + " ";
    int lPad = (totalW - static_cast<int>(title.size())) / 2;
    std::cout << BOX_V << std::string(lPad, ' ') << title
              << std::string(totalW - lPad - static_cast<int>(title.size()), ' ') << BOX_V "\n";

    std::string sub = " " + dayName + " ";
    lPad = (totalW - static_cast<int>(sub.size())) / 2;
    std::cout << BOX_V << std::string(lPad, ' ') << sub
              << std::string(totalW - lPad - static_cast<int>(sub.size()), ' ') << BOX_V "\n";

    std::cout << BOX_BL << std::string(totalW, BOX_H[0]) << BOX_BR "\n";
    sleepMs(500);
}

void ConsoleUI::PrintDayHeader(int day, const std::string& dayName) {
    ShowDayTransition(day, dayName);
}

void ConsoleUI::PrintStatus(const Player& player) {
    std::cout << BOX_V " [" << player.getTimeString() << "] "
              << locationToString(player.getLocation())
              << " | EN: " << player.getStats().energy
              << " FAT: " << player.getStats().fatigue << "\n";
}

void ConsoleUI::PrintText(const std::string& text) {
    std::cout << BOX_V " " << text << "\n";
}

void ConsoleUI::PrintLocationMenu(LocationID location) {
    int totalW = UIModeManager::screenW();
    std::cout << BOX_TL << std::string(totalW, BOX_H[0]) << BOX_TR "\n";
    std::cout << BOX_V " " << rpad(locationToString(location), totalW - 2) << " " BOX_V "\n";
    std::cout << BOX_L << std::string(totalW, BOX_H[0]) << BOX_R "\n";
}

void ConsoleUI::ShowPlayerStatsPanel(const Player& player) {
    PrintPlayerStats(player);
}

void ConsoleUI::ShowExamPanel(const std::string& examName, int score) {
    int totalW = UIModeManager::screenW();
    std::cout << BOX_TL << std::string(totalW, BOX_H[0]) << BOX_TR "\n";
    std::cout << BOX_V " " << rpad("Экзамен: " + examName, totalW - 2) << " " BOX_V "\n";
    std::cout << BOX_V " " << rpad("Результат: " + std::to_string(score) + "/100", totalW - 2) << " " BOX_V "\n";
    std::cout << BOX_BL << std::string(totalW, BOX_H[0]) << BOX_BR "\n";
}

// ---- Animation wrappers ----

void ConsoleUI::TypeText(const std::string& text, int speedMs) {
    for (size_t i = 0; i < text.size(); i++) {
        std::cout << text[i] << std::flush;
        sleepMs(speedMs);
    }
}

// ---- Safe Animations ----

void ConsoleUI::TypeLine(const std::string& line, int speedMs) {
    for (size_t i = 0; i < line.size(); i++) {
        std::cout << line[i] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(speedMs));
    }
    std::cout << "\n";
}

void ConsoleUI::AnimateMenuItems(const std::vector<std::string>& items, int w) {
    for (const auto& item : items) {
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        std::cout << BOX_V " " << rpad(item, w - 2) << " " BOX_V "\n";
    }
}

void ConsoleUI::AnimateLogoLine(const std::string& line, int w, int speedMs) {
    size_t vlen = visLen(line);
    int l = (w - static_cast<int>(vlen)) / 2;
    if (l < 0) l = 0;
    int pad = w - l - static_cast<int>(vlen);
    if (pad < 0) pad = 0;
    std::cout << BOX_V;
    for (int i = 0; i < l; i++) {
        std::cout << ' ';
    }
    for (size_t i = 0; i < line.size(); i++) {
        std::cout << line[i] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(speedMs));
    }
    for (int i = 0; i < pad; i++) {
        std::cout << ' ';
    }
    std::cout << BOX_V "\n";
}

void ConsoleUI::AnimateRain(int lines, int speedMs) {
    int w = UIModeManager::screenW();
    for (int i = 0; i < lines; i++) {
        for (int j = 0; j < w; j++) {
            if (rand() % 5 == 0) std::cout << "|";
            else std::cout << " ";
        }
        std::cout << "\n" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(speedMs));
    }
}

void ConsoleUI::AnimatePhoneCall() {
    for (int i = 0; i < 3; i++) {
        std::cout << "\r   ЗВОНОК   " << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "\r            " << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "\r            \r";
}

void ConsoleUI::AnimateExamResult(bool success) {
    if (success) {
        for (int i = 0; i < 3; i++) {
            std::cout << "\r  СДАНО!  " << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << "\r           " << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
        std::cout << "\r  СДАНО!\n";
    } else {
        std::cout << "  ПРОВАЛ...\n";
    }
}

void ConsoleUI::AnimateHeart(int w) {
    std::string heart = " <3 ";
    int l = (w - static_cast<int>(visLen(heart))) / 2;
    if (l < 0) l = 0;
    for (int i = 0; i < 5; i++) {
        std::cout << "\r" << std::string(l, ' ') << heart << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "\r" << std::string(l + static_cast<int>(heart.size()), ' ') << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    std::cout << "\r" << std::string(l, ' ') << heart << "\n";
}

int ConsoleUI::GetChoiceWithCursor(int maxChoice) {
    (void)maxChoice;
    std::cout << "> " << std::flush;
    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');
    return choice;
}

// ---- Backward-compat wrappers ----

std::string ConsoleUI::GetNPCPortrait(const std::string& npcName) {
    return AsciiArt::ForNPC(npcName);
}

std::string ConsoleUI::GetAllaPortrait() { return AsciiArt::GetAlla(); }
std::string ConsoleUI::GetBulatPortrait() { return AsciiArt::GetBulat(); }
std::string ConsoleUI::GetSemenPortrait() { return AsciiArt::GetSemen(); }
std::string ConsoleUI::GetArtemPortrait() { return AsciiArt::GetArtem(); }

std::string ConsoleUI::GetLocationArt(LocationID loc) {
    return AsciiArt::ForLocation(static_cast<int>(loc));
}

void ConsoleUI::ShowNPCPortrait(const std::string& npcName) {
    std::cout << AsciiArt::ForNPC(npcName);
}

void ConsoleUI::ShowLocationArt(LocationID loc) {
    std::cout << AsciiArt::ForLocation(static_cast<int>(loc));
}

std::string ConsoleUI::MakeRightPanel(const Player& player,
                                       const std::string& npcName,
                                       const std::string& additionalInfo) {
    (void)additionalInfo;
    return AutoRightPanel(player, npcName);
}
