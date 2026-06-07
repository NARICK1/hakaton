#pragma once
#include "../player/Player.h"
#include "../data/UIMode.h"
#include <string>
#include <vector>
#include <limits>

// Pure ASCII box drawing
#define BOX_H  "="
#define BOX_V  "|"
#define BOX_TL "+"
#define BOX_TR "+"
#define BOX_BL "+"
#define BOX_BR "+"
#define BOX_L  "+"
#define BOX_R  "+"
#define BOX_X  "+"

#define BAR_EMPTY "."
#define BAR_HALF  ":"
#define BAR_FULL  "#"
#define BAR_SOLID "#"

struct UIPanel {
    int x, y, width, height;
    std::string title;
};

// UTF-8 aware helpers
size_t visLen(const std::string& s);
std::string rpad(const std::string& s, int visWidth);
std::vector<std::string> wordWrap(const std::string& text, int visWidth);

class ConsoleUI {
public:
    static void SetConsoleUTF8();
    static void ClearScreen();
    static void WaitForEnter();

    // Запоминает диапазон выбора для следующего ReadInt() без явных min/max.
    // Нужно для кат-сцен и ручных диалогов, где варианты уже напечатаны на экране.
    static void SetPendingChoiceRange(int minValue, int maxValue);
    static void ClearPendingChoiceRange();

    // Безопасный ввод числа.
    // Не ломает cin, если игрок ввёл букву или мусор.
    static int ReadInt(
        const std::string& prompt = "",
        int minValue = std::numeric_limits<int>::min(),
        int maxValue = std::numeric_limits<int>::max()
    );

    static int ShowMenu(const std::vector<std::string>& options,
                        const std::string& prompt = "Ваш выбор");

    // Unified screen renderer with auto right panel
    static void RenderScreen(const std::string& sceneTitle,
                             const std::string& mainText,
                             const std::vector<std::string>& choices,
                             const Player& player,
                             const std::string& rightPanel = "",
                             const std::string& sceneNPC = "",
                             const std::string& sceneLocation = "",
                             bool isExam = false,
                             bool isRain = false,
                             bool isEnding = false,
                             bool endingSuccess = false);

    // Specialized screens
    static void ShowPlayerStatsPanel(const Player& player);
    static void ShowExamPanel(const std::string& examName, int score);

    // Legacy text helpers
    static void PrintHeader(const std::string& title);
    static void PrintSeparator();
    static void PrintPlayerStats(const Player& player);
    static void PrintDayHeader(int day, const std::string& dayName);
    static void PrintStatus(const Player& player);
    static void PrintText(const std::string& text);
    static void PrintLocationMenu(LocationID location);

    // Right panel auto-builder
    static std::string AutoRightPanel(const Player& player,
                                       const std::string& npcName = "",
                                       const std::string& locationName = "",
                                       bool isExam = false,
                                       bool isRain = false,
                                       bool isEnding = false,
                                       bool endingSuccess = false);

    // Backward-compat: ASCII art access
    static std::string GetNPCPortrait(const std::string& npcName);
    static std::string GetAllaPortrait();
    static std::string GetBulatPortrait();
    static std::string GetSemenPortrait();
    static std::string GetArtemPortrait();
    static std::string GetLocationArt(LocationID loc);
    static void ShowNPCPortrait(const std::string& npcName);
    static void ShowLocationArt(LocationID loc);
    static std::string MakeRightPanel(const Player& player,
                                       const std::string& npcName = "",
                                       const std::string& additionalInfo = "");

    // Status bars
    static std::string MakeBarString(int value, int maxVal, int width);
    static std::string MakeTopBar(const Player& player);
    static std::string MakeStatBar(const Player& player);

    // Animations
    static void TypeText(const std::string& text, int speedMs = 5);
    static void TypeLine(const std::string& line, int speedMs = 10);
    static void ShowDayTransition(int day, const std::string& dayName);
    static void AnimateMenuItems(const std::vector<std::string>& items, int w);
    static void AnimateLogoLine(const std::string& line, int w, int speedMs = 30);
    static void AnimateRain(int lines = 3, int speedMs = 80);
    static void AnimatePhoneCall();
    static void AnimateExamResult(bool success);
    static void AnimateHeart(int w);
    static int GetChoiceWithCursor(int maxChoice);
};