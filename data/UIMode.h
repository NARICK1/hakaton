#pragma once
#include <string>
#include <fstream>

struct UIProfile {
    int screenWidth;
    int mainWidth;
    int rightPanelWidth;
    int hudLines;
    int storyRows;
    int dialogRows;
    std::string name;
    static UIProfile Compact() { return {78, 48, 24, 1, 6, 3, "Компактный"}; }
    static UIProfile Standard() { return {118, 72, 40, 2, 10, 5, "Стандартный"}; }
    static UIProfile Wide() { return {138, 84, 48, 2, 14, 6, "Широкий"}; }
    static UIProfile Default() { return Standard(); }
};

class UIModeManager {
private:
    UIProfile current;
    int language;
    static UIModeManager& instance() {
        static UIModeManager inst;
        return inst;
    }
    UIModeManager() : current(UIProfile::Default()), language(0) { loadProfile(); }
    std::string profilePath() { return "profile.cfg"; }
    void saveProfile() {
        std::ofstream f(profilePath());
        if (f.is_open()) {
            std::string m;
            if (current.screenWidth == UIProfile::Compact().screenWidth) m = "compact";
            else if (current.screenWidth == UIProfile::Wide().screenWidth) m = "wide";
            else m = "standard";
            f << m << "\n" << (language == 0 ? "russian" : "english") << "\n";
        }
    }
    void loadProfile() {
        std::ifstream f(profilePath());
        if (f.is_open()) {
            std::string mode; std::getline(f, mode);
            if (mode == "compact") current = UIProfile::Compact();
            else if (mode == "wide") current = UIProfile::Wide();
            else current = UIProfile::Standard();
            std::string lang; std::getline(f, lang);
            if (lang == "english") language = 1;
        }
    }
public:
    static const UIProfile& get() { return instance().current; }
    static void setCompact() { instance().current = UIProfile::Compact(); instance().saveProfile(); }
    static void setStandard() { instance().current = UIProfile::Standard(); instance().saveProfile(); }
    static void setWide() { instance().current = UIProfile::Wide(); instance().saveProfile(); }
    static bool isCompact() { return instance().current.screenWidth == UIProfile::Compact().screenWidth; }
    static bool isWide() { return instance().current.screenWidth == UIProfile::Wide().screenWidth; }
    static int screenW() { return instance().current.screenWidth; }
    static int mainW() { return instance().current.mainWidth; }
    static int rpW() { return instance().current.rightPanelWidth; }
    static int storyRows() { return instance().current.storyRows; }
    static int dialogRows() { return instance().current.dialogRows; }
    static std::string currentName() { return instance().current.name; }
    static int getLang() { return instance().language; }
    static void setLang(int l) { instance().language = (l == 0 ? 0 : 1); instance().saveProfile(); }
    static bool isRussian() { return instance().language == 0; }
};
