#include "DevMode.h"
#include "ConsoleUI.h"
#include "Menu.h"
#include "../data/UIMode.h"
#include "../data/Lang.h"
#include "../data/AsciiArt.h"
#include "../npc/NPC.h"
#include "../systems/EndingSystem.h"
#include "../core/GameState.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

static int W() { return UIModeManager::screenW(); }
static void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ---- Helpers ----

void DevMode::showSubheader(const std::string& title) {
    int w = W();
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  ◈ " << rpad(title, w - 6) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);
}

void DevMode::waitEnter() {
    std::cout << "\n " << Lang::get("ui_press_enter") << " " << std::flush;
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

void DevMode::glitchEffect(int lines) {
    int w = W();
    for (int i = 0; i < lines; i++) {
        for (int j = 0; j < w; j++) {
            if (rand() % 3 == 0) std::cout << char(rand() % 94 + 33);
            else std::cout << " ";
        }
        std::cout << "\r" << std::flush;
        sleepMs(30);
    }
    std::cout << std::string(w, ' ') << "\r" << std::flush;
    sleepMs(50);
}

void DevMode::loadingDemo() {
    int w = W();
    int barLen = (w < 60 ? w - 10 : 50);
    for (int i = 0; i <= 20; i++) {
        std::cout << "\r " << Lang::get("menu_title") << " [";
        int filled = (i * barLen) / 20;
        for (int j = 0; j < barLen; j++) {
            std::cout << (j < filled ? "#" : ".");
        }
        std::cout << "] " << (i * 5) << "%" << std::flush;
        sleepMs(40);
    }
    std::cout << "\n";
}

// ---- Главное меню режима разработчика ----

void DevMode::ShowDevMenu() {
    bool running = true;
    while (running) {
        ConsoleUI::ClearScreen();
        int w = W();
        ConsoleUI::PrintHeader("РЕЖИМ РАЗРАБОТЧИКА", DECO_EVENT);
        std::cout << BOX_V "  " << rpad("Позволяет просматривать технические возможности игры:", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 1.  Демо анимаций", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 2.  Галерея персонажей", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 3.  Галерея локаций", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 4.  Комикс", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 5.  Демонстрация интерфейсов", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 6.  Галерея концовок", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 7.  Симуляция памяти NPC", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 8.  Симуляция отношений", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 9.  Демонстрация событий", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("◈ 10. Бенчмарк движка", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::cout << BOX_V "  " << rpad("0.  Назад", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        std::cout << " " << Lang::get("ui_your_choice") << ": " << std::flush;

        int ch;
        std::cin >> ch;
        std::cin.ignore(10000, '\n');

        switch (ch) {
        case 1: DemoAnimations(); break;
        case 2: GalleryCharacters(); break;
        case 3: GalleryLocations(); break;
        case 4: ShowComic(); break;
        case 5: DemoInterfaces(); break;
        case 6: GalleryEndings(); break;
        case 7: SimulateNPCMemory(); break;
        case 8: SimulateRelations(); break;
        case 9: DemoEvents(); break;
        case 10: Benchmark(); break;
        case 0: running = false; break;
        default: break;
        }
    }
}

// ---- 1. Демо анимаций ----

void DevMode::DemoAnimations() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader("ДЕМО АНИМАЦИЙ", DECO_EVENT);
    int w = W();

    std::cout << BOX_V "  " << rpad("1. Плавная печать текста", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Этот текст печатается по символам...", w - 4) << "  " BOX_V "\n" << std::flush;
    sleepMs(300);
    // Типографический эффект
    std::string demoText = "Привет, мир консольной анимации! Каждый символ появляется с задержкой.";
    ConsoleUI::TypeLine(demoText, 15);
    sleepMs(200);

    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("2. Эффект глитча", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    glitchEffect(5);
    sleepMs(100);

    std::cout << BOX_V "  " << rpad("3. Демо загрузки", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V << " ";
    loadingDemo();
    sleepMs(100);

    std::cout << BOX_V "  " << rpad("4. Дождь", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::AnimateRain(4, 60);
    sleepMs(100);

    std::cout << BOX_V "  " << rpad("5. Телефонный звонок", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V << " ";
    ConsoleUI::AnimatePhoneCall();
    sleepMs(100);

    std::cout << BOX_V "  " << rpad("6. Результат экзамена (успех)", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V << " ";
    ConsoleUI::AnimateExamResult(true);
    sleepMs(100);

    std::cout << BOX_V "  " << rpad("7. Результат экзамена (провал)", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V << " ";
    ConsoleUI::AnimateExamResult(false);
    sleepMs(100);

    std::cout << BOX_V "  " << rpad("8. Анимация <3", w - 4) << "  " << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::AnimateHeart(w);

    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);
    waitEnter();
}

// ---- 2. Галерея персонажей ----

void DevMode::GalleryCharacters() {
    struct CharInfo {
        std::string name;
        std::string desc;
        std::string type;
        std::string role;
        std::string (*art)();
    };
    CharInfo chars[] = {
        {"Алла", "Одногруппница Тимура, умная и красивая девушка. Отличница, но скромная.",
         "Дружелюбный, романтичный", "Основной NPC, романтическая линия", AsciiArt::GetAlla},
        {"Булат", "Лучший друг Тимура, весёлый и надёжный парень. Всегда готов прийти на помощь.",
         "Дружелюбный, экстраверт", "Основной NPC, поддержка", AsciiArt::GetBulat},
        {"Семён", "Студент-старшекурсник, циничный и умный. Знает все ходы и выходы.",
         "Циничный, прагматичный", "Основной NPC, информатор", AsciiArt::GetSemen},
        {"Артём", "Замкнутый программист, гений математики. Почти не общается, но гениален.",
         "Интроверт, гений", "Основной NPC, помощь с учёбой", AsciiArt::GetArtem}
    };

    for (const auto& c : chars) {
        ConsoleUI::ClearScreen();
        int w = W();
        ConsoleUI::PrintHeader("ГАЛЕРЕЯ ПЕРСОНАЖЕЙ", DECO_EVENT);
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

        // ASCII art
        std::string art = c.art();
        std::istringstream artStream(art);
        std::string line;
        while (std::getline(artStream, line)) {
            int lPad = (w - static_cast<int>(visLen(line))) / 2;
            if (lPad < 0) lPad = 0;
            std::cout << BOX_V << std::string(lPad, ' ') << line
                      << std::string(w - lPad - static_cast<int>(visLen(line) > 0 ? visLen(line) : 0), ' ') << BOX_V "\n";
        }

        ConsoleUI::PrintSeparator(DECO_EVENT);
        std::cout << BOX_V "  " << rpad("Имя: " + c.name, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Тип: " + c.type, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Роль: " + c.role, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::cout << BOX_V "  " << rpad(c.desc, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        waitEnter();
    }
}

// ---- 3. Галерея локаций ----

void DevMode::GalleryLocations() {
    struct LocInfo {
        std::string name;
        std::string desc;
        std::string usage;
        std::string (*art)();
    };
    LocInfo locs[] = {
        {"Общежитие", "Комната в студенческом общежитии. Тимур живёт здесь.",
         "Начало и конец каждого дня", AsciiArt::GetHome},
        {"Университет", "Главное здание Уфимского университета науки и технологий.",
         "Экзамены, встречи с NPC", AsciiArt::GetUniversity},
        {"Аудитория", "Учебная аудитория. Здесь проходят экзамены и лекции.",
         "Экзамены по всем предметам", AsciiArt::GetClassroom},
        {"Столовая", "Университетская столовая. Недорого, но вкусно.",
         "Восстановление голода", AsciiArt::GetCafeteria},
        {"Улица", "Городские улицы Уфы. Оживлённое движение, прохожие.",
         "Перемещение между локациями", AsciiArt::GetStreet}
    };

    for (const auto& l : locs) {
        ConsoleUI::ClearScreen();
        int w = W();
        ConsoleUI::PrintHeader("ГАЛЕРЕЯ ЛОКАЦИЙ", DECO_EVENT);
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

        std::string art = l.art();
        std::istringstream artStream(art);
        std::string line;
        while (std::getline(artStream, line)) {
            int lPad = (w - static_cast<int>(visLen(line))) / 2;
            if (lPad < 0) lPad = 0;
            std::cout << BOX_V << std::string(lPad, ' ') << line
                      << std::string(w - lPad - static_cast<int>(visLen(line) > 0 ? visLen(line) : 0), ' ') << BOX_V "\n";
        }

        ConsoleUI::PrintSeparator(DECO_EVENT);
        std::cout << BOX_V "  " << rpad("Локация: " + l.name, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Использование: " + l.usage, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::cout << BOX_V "  " << rpad(l.desc, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        waitEnter();
    }
}

// ---- 4. Комикс ----

void DevMode::ShowComic() {
    struct ComicFrame {
        std::string title;
        std::string text;
        std::string art;
        int delayMs;
    };

    ComicFrame frames[] = {
        {"FRAME 1: MORNING",
         "Timur wakes up in his room. The city is noisy outside.",
         AsciiArt::GetHome(), 1000},
        {"FRAME 2: ROAD",
         "He quickly gets ready and runs outside.",
         AsciiArt::GetStreet(), 1000},
        {"FRAME 3: UNIVERSITY",
         "The university building towers before him.",
         AsciiArt::GetUniversity(), 1000},
        {"FRAME 4: CLASSROOM",
         "The exam begins. Timur takes a ticket.",
         AsciiArt::GetClassroom(), 1200},
        {"FRAME 5: BREAK",
         "After the exam he meets Alla in the hallway.",
         AsciiArt::GetAlla(), 1000},
        {"FRAME 6: CAFETERIA",
         "Friends discuss results over lunch.",
         AsciiArt::GetCafeteria(), 1200},
        {"FRAME 7: SUNSET",
         "Timur returns home. It was a tough day but he made it.",
         AsciiArt::GetHome(), 1500}
    };

    // Сначала выбираем режим
    ConsoleUI::ClearScreen();
    int w = W();
    ConsoleUI::PrintHeader("COMIC MODE", DECO_EVENT);
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V << rpad("  Choose viewing mode:", w) << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V << rpad("  1. Manual (ENTER for next frame)", w) << BOX_V "\n";
    std::cout << BOX_V << rpad("  2. Auto (automatic slideshow)", w) << BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << "> ";
    int modeChoice;
    std::cin >> modeChoice;
    std::cin.ignore(10000, '\n');
    bool manualMode = (modeChoice == 1);

    int current = 0;
    while (current < 7) {
        ConsoleUI::ClearScreen();
        ConsoleUI::PrintHeader("COMIC — " + frames[current].title, DECO_EVENT);

        std::string art = frames[current].art;
        std::istringstream artStream(art);
        std::string line;
        while (std::getline(artStream, line)) {
            size_t v = visLen(line);
            int lPad = (w - static_cast<int>(v)) / 2;
            if (lPad < 0) lPad = 0;
            int rPad = w - lPad - static_cast<int>(v);
            if (rPad < 0) rPad = 0;
            std::cout << BOX_V << std::string(lPad, ' ') << line
                      << std::string(rPad, ' ') << BOX_V "\n";
        }
        ConsoleUI::PrintSeparator(DECO_EVENT);
        std::cout << BOX_V "  " << rpad(frames[current].text, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::cout << BOX_V "  " << rpad("[" + std::to_string(current + 1) + "/7] Q - quit",
                                        w - 4) << "  " BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);

        if (manualMode) {
            std::string cmd;
            std::getline(std::cin, cmd);
            if (cmd == "q" || cmd == "Q") break;
            current++;
        } else {
            sleepMs(frames[current].delayMs);
            current++;
        }
    }

    ConsoleUI::WaitForEnter();
}

// ---- 5. Демонстрация интерфейсов ----

void DevMode::DemoInterfaces() {
    struct ProfileDemo {
        std::string name;
        void (*setter)();
    };
    ProfileDemo profiles[] = {
        {"Компактный (80 символов)", UIModeManager::setCompact},
        {"Стандартный (120 символов)", UIModeManager::setStandard},
        {"Широкий (140 символов)", UIModeManager::setWide}
    };

    for (const auto& p : profiles) {
        p.setter();
        ConsoleUI::ClearScreen();
        int w = W();
        ConsoleUI::PrintHeader("ДЕМОНСТРАЦИЯ: " + p.name, DECO_EVENT);

        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Текущий профиль: " + UIModeManager::currentName(), w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Ширина экрана: " + std::to_string(w) + " символов", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Основная колонка: " + std::to_string(UIModeManager::mainW()) + " символов", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Правая панель: " + std::to_string(UIModeManager::rpW()) + " символов", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Строк сюжета: " + std::to_string(UIModeManager::storyRows()), w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("Строк диалога: " + std::to_string(UIModeManager::dialogRows()), w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);

        // HUD Demo
        std::cout << BOX_V << rpad(" ДЕНЬ: 1 | ВРЕМЯ: 09:00 | ЛОКАЦИЯ: Университет", w) << BOX_V "\n";
        std::cout << BOX_V << rpad(" ИНТ:75 ЭН:50 УСТ:20 ГОЛ:10 СТР:30 РУБ:1200", w) << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);

        // Story block
        std::cout << BOX_V << rpad(" Добро пожаловать в демонстрацию интерфейсов!", w) << BOX_V "\n";
        std::cout << BOX_V << rpad(" Этот экран показывает, как выглядит каждый режим.", w) << BOX_V "\n";
        std::cout << BOX_V << rpad("", w) << BOX_V "\n";
        for (int i = 0; i < UIModeManager::storyRows() - 3; i++) {
            std::cout << BOX_V << rpad(" ~ строка сюжета ~", w) << BOX_V "\n";
        }

        ConsoleUI::PrintSeparator(DECO_EVENT);
        std::cout << BOX_V << rpad(" [NPC] Привет! Как дела?", w) << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        std::cout << BOX_V << rpad(" 1. Поздороваться", w) << BOX_V "\n";
        std::cout << BOX_V << rpad(" 2. Спросить про экзамен", w) << BOX_V "\n";
        std::cout << BOX_V << rpad(" 3. Попрощаться", w) << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);

        waitEnter();
    }

    // Restore standard
    UIModeManager::setStandard();
}

// ---- 6. Галерея концовок ----

void DevMode::GalleryEndings() {
    struct EndingDemo {
        std::string title;
        std::string body;
    };

    EndingDemo endings[] = {
        {"СУПЕР-КОНЦОВКА",
         "Тимур сдал все экзамены на отлично! Профессора поражены.\nДрузья гордятся. Алла и Тимур по-настоящему влюблены.\nКрасный диплом и предложение от лучшей IT-компании."},
        {"ХОРОШАЯ КОНЦОВКА",
         "Тимур успешно сдал сессию. Не всё гладко, но он справился.\nЛето, каникулы и новые планы. Жизнь продолжается!"},
        {"НОРМАЛЬНАЯ КОНЦОВКА",
         "Сессия закончена. Взлёты и падения. Тимур выжил.\nВпереди ещё много семестров. В целом всё нормально."},
        {"ОТЧИСЛЕНИЕ",
         "Тимура отчислили за неуспеваемость. Слишком много долгов.\nРодители разочарованы. Будущее туманно."},
        {"АКАДЕМИЧЕСКИЙ ОТПУСК",
         "Нервы сдали. Тимур взял академ. Может, вернётся через год.\nГлавное — отдохнуть и восстановиться."},
        {"ПСИХБОЛЬНИЦА",
         "Стресс сломал Тимура. Бессонные ночи, постоянное напряжение.\nТеперь нужна профессиональная помощь."},
        {"СМЕРТЬ ОТ ГОЛОДА",
         "Тимур забывал поесть так часто, что организм не выдержал.\nВажно было вовремя обедать..."},
        {"ВЕЧНЫЙ ДОЛЖНИК",
         "Долги растут. Сессия провалена, хвосты множатся.\nЕщё один семестр... И ещё... И ещё..."},
        {"СЕКРЕТНАЯ РОМАНТИЧЕСКАЯ",
         "Тимур и Алла — идеальная пара. Вместе сдали экзамены.\nИ жили они долго и счастливо... до следующей сессии."},
        {"СЕКРЕТНАЯ АРМЕЙСКАЯ",
         "Тимур решил, что учёба — не его. Армия ждёт!\nДва года службы, новые друзья, новые испытания."}
    };

    for (const auto& e : endings) {
        ConsoleUI::ClearScreen();
        int w = W();
        std::string t = "✦ " + e.title + " ✦";
        ConsoleUI::PrintHeader(t, DECO_EVENT);

        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::vector<std::string> wrapped = wordWrap(e.body, w - 4);
        for (const auto& line : wrapped) {
            std::cout << BOX_V "  " << rpad(line, w - 4) << "  " BOX_V "\n";
        }
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

        // Decorative footer
        for (int i = 0; i < w; i++) {
            int r = (i % 3 == 0) ? 3 : 0;
            if (r == 3) std::cout << "~";
            else std::cout << " ";
        }
        std::cout << "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        waitEnter();
    }
}

// ---- 7. Симуляция памяти NPC ----

void DevMode::SimulateNPCMemory() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader("СИМУЛЯЦИЯ ПАМЯТИ NPC", DECO_EVENT);
    int w = W();

    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Демонстрация того, как NPC запоминают действия игрока:", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    struct DemoEvent {
        std::string action;
        std::string category;
        std::string npcReaction;
        int relChange;
        int trustChange;
    };

    DemoEvent events[] = {
        {"помог с учёбой", "helpful", "«Спасибо, ты настоящий друг!»", +10, +5},
        {"нагрубил в разговоре", "rude", "«... (обиженно молчит)»", -8, -4},
        {"сделал комплимент", "romance", "«Ты такой милый...»", +12, +3},
        {"нарушил обещание", "selfish", "«Я больше тебе не верю.»", -15, -10},
        {"поддержал в трудный момент", "trust", "«Ты всегда можешь на меня рассчитывать.»", +15, +10},
        {"подарил подарок", "generous", "«Ой, спасибо! Как неожиданно!»", +10, +5},
        {"отказал в помощи", "selfish", "«Ну как хочешь...»", -5, -3}
    };

    int rel = 50, trust = 50;
    for (const auto& e : events) {
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        std::cout << BOX_V "  " << rpad("→ Действие: игрок " + e.action, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("  Категория: " + e.category, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("  Реакция NPC: " + e.npcReaction, w - 4) << "  " BOX_V "\n";

        rel = std::clamp(rel + e.relChange, 0, 100);
        trust = std::clamp(trust + e.trustChange, 0, 100);

        std::cout << BOX_V "  " << rpad("  Отношение: " + std::to_string(rel) + "/100"
                                          " | Доверие: " + std::to_string(trust) + "/100", w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        sleepMs(500);
    }

    ConsoleUI::PrintSeparator(DECO_EVENT);
    waitEnter();
}

// ---- 8. Симуляция отношений ----

void DevMode::SimulateRelations() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader("СИМУЛЯЦИЯ ОТНОШЕНИЙ", DECO_EVENT);
    int w = W();

    struct RelLayer {
        std::string name;
        int value;
        std::string desc;
    };

    RelLayer layers[] = {
        {"Отношение", 60, "Общий уровень симпатии. Влияет на базовое поведение NPC."},
        {"Доверие", 45, "Готовность делиться секретами и помогать в сложных ситуациях."},
        {"Уважение", 55, "Признание заслуг игрока. Влияет на готовность слушать советы."},
        {"Симпатия", 70, "Эмоциональная привязанность. Важно для романтической линии."},
        {"Интерес", 40, "Желание общаться и проводить время вместе."},
        {"Раздражение", 25, "Накопленная негативная эмоция. Снижает все остальные параметры."}
    };

    for (int step = 0; step < 3; step++) {
        ConsoleUI::ClearScreen();
        std::string phase;
        if (step == 0) phase = "НАЧАЛЬНОЕ СОСТОЯНИЕ";
        else if (step == 1) phase = "ПОСЛЕ ПОЛОЖИТЕЛЬНЫХ ДЕЙСТВИЙ";
        else phase = "ПОСЛЕ ОТРИЦАТЕЛЬНЫХ ДЕЙСТВИЙ";
        ConsoleUI::PrintHeader("СИМУЛЯЦИЯ — " + phase, DECO_EVENT);

        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        for (const auto& l : layers) {
            int val = l.value;
            if (step == 1) {
                if (l.name == "Отношение") val = 80;
                else if (l.name == "Доверие") val = 70;
                else if (l.name == "Уважение") val = 75;
                else if (l.name == "Симпатия") val = 85;
                else if (l.name == "Интерес") val = 65;
                else if (l.name == "Раздражение") val = 10;
            } else if (step == 2) {
                if (l.name == "Отношение") val = 30;
                else if (l.name == "Доверие") val = 15;
                else if (l.name == "Уважение") val = 25;
                else if (l.name == "Симпатия") val = 20;
                else if (l.name == "Интерес") val = 10;
                else if (l.name == "Раздражение") val = 80;
            }

            std::string bar = ConsoleUI::MakeBarString(val, 100, 30);
            std::cout << BOX_V "  " << rpad(l.name + ": " + std::to_string(val) + "/100 " + bar, w - 4) << "  " BOX_V "\n";
        }
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        std::cout << BOX_V "  " << rpad("Пример поведения NPC при текущих параметрах:", w - 4) << "  " BOX_V "\n";

        if (step == 0)
            std::cout << BOX_V "  " << rpad("NPC ведёт себя нейтрально, готов к диалогу.", w - 4) << "  " BOX_V "\n";
        else if (step == 1)
            std::cout << BOX_V "  " << rpad("NPC дружелюбен, делится информацией, помогает.", w - 4) << "  " BOX_V "\n";
        else
            std::cout << BOX_V "  " << rpad("NPC избегает контакта, отказывает в помощи.", w - 4) << "  " BOX_V "\n";

        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        waitEnter();
    }
}

// ---- 9. Демонстрация событий ----

void DevMode::DemoEvents() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader("ДЕМОНСТРАЦИЯ СОБЫТИЙ", DECO_EVENT);
    int w = W();

    struct RandEvent {
        std::string title;
        std::string desc;
        std::string effect;
    };

    RandEvent events[] = {
        {"Найдены деньги", "Ты нашёл 500 рублей на улице!", "+500 руб"},
        {"Потеря денег", "Ты потерял кошелёк...", "-300 руб"},
        {"Встреча знакомого", "Старый друг угощает тебя обедом.", "+100 руб, +5 энергии"},
        {"Скидка на еду", "В столовой сегодня акция!", "-50% на обед"},
        {"Разговор с преподавателем", "Препод даёт ценный совет по экзамену.", "+5 интеллекта"},
        {"Опоздание на пару", "Ты проспал и опоздал на лекцию.", "-10 энергии"},
        {"Анекдот от Семёна", "«Идёт программист по улице...»", "+5 настроения"},
        {"Редкое событие: падающая звезда", "Ты загадал желание. Сбудется ли?", "???..."}
    };

    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Генератор случайных событий:", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    srand(static_cast<unsigned>(time(nullptr)));
    for (int i = 0; i < 5; i++) {
        int idx = rand() % 8;
        const auto& e = events[idx];

        std::cout << BOX_V "  " << rpad("✦ " + e.title, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("  " + e.desc, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V "  " << rpad("  Эффект: " + e.effect, w - 4) << "  " BOX_V "\n";
        std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
        ConsoleUI::PrintSeparator(DECO_EVENT);
        sleepMs(300);
    }

    ConsoleUI::PrintSeparator(DECO_EVENT);
    waitEnter();
}

// ---- 10. Бенчмарк движка ----

void DevMode::Benchmark() {
    ConsoleUI::ClearScreen();
    ConsoleUI::PrintHeader("БЕНЧМАРК ДВИЖКА / ОТЛАДОЧНАЯ СВОДКА", DECO_EVENT);
    int w = W();

    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << BOX_V "  " << rpad("◆ СТАТИСТИКА ПРОЕКТА", w - 4) << "  " BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << BOX_V "  " << rpad("Количество NPC: 4 (Алла, Булат, Семён, Артём)", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Количество дней: 8", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Количество экзаменов: 5", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Количество концовок: 10", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Количество анимаций: 9", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Количество режимов интерфейса: 3", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << BOX_V "  " << rpad("◆ ТЕКУЩАЯ КОНФИГУРАЦИЯ", w - 4) << "  " BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << BOX_V "  " << rpad("Профиль интерфейса: " + UIModeManager::currentName(), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Ширина экрана: " + std::to_string(UIModeManager::screenW()) + " пикселей (псевдо)", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Язык: " + Lang::currentName(), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << BOX_V "  " << rpad("◆ ТЕХНИЧЕСКАЯ ИНФОРМАЦИЯ", w - 4) << "  " BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << BOX_V "  " << rpad("Язык разработки: C++17", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Компилятор: MSVC 19.51 (x64)", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Сторонние библиотеки: отсутствуют", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Графический движок: отсутствует (чистая консоль)", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Аудио: отсутствует", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Рендер: ASCII + UTF-8 псевдографика", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Локализация: Русский / English (250+ строк)", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Сохранения: бинарный файл savegame.dat", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    ConsoleUI::PrintSeparator(DECO_EVENT);
    std::cout << BOX_V "  " << rpad("◆ ТЕСТ РЕНДЕРА", w - 4) << "  " BOX_V "\n";
    ConsoleUI::PrintSeparator(DECO_EVENT);

    // Quick perf test
    int boxCount = 0;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 50; i++) {
        std::string testLine = BOX_V " " + rpad("Тестовая строка рендера #" + std::to_string(i + 1), w - 4) + " " BOX_V "\n";
        boxCount++;
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << BOX_V "  " << rpad("Сгенерировано строк: " + std::to_string(boxCount), w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Время генерации: " + std::to_string(elapsed) + " мс", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Производительность: высокая", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << std::string(w, ' ') << BOX_V "\n";

    ConsoleUI::PrintSeparator(DECO_EVENT);
    waitEnter();
}
