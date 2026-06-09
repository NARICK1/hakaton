#include "Game.h"
#include "../ui/ConsoleUI.h"
#include "../ui/Menu.h"
#include "../ui/DevMode.h"
#include "../ui/Credits.h"
#include "../systems/EndingSystem.h"
#include "../systems/RelationshipSystem.h"
#include "../systems/FatigueSystem.h"
#include "../systems/HungerSystem.h"
#include "../systems/DebuffSystem.h"
#include "../exams/Exam.h"
#include "../data/Lang.h"
#include "../data/Constants.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>
#include <vector>

namespace {
    int scaledCost(const Player& player, int baseCost) {
        return player.scaleCost(baseCost);
    }

    void gainIntellect(Player& player, int value) {
        player.getStats().intellect += player.scaleGain(value);
    }

    void gainEnergy(Player& player, int value) {
        player.getStats().energy += player.scaleGain(value);
    }

    void loseEnergy(Player& player, int value) {
        player.getStats().energy -= player.scalePenalty(value);
    }

    void gainFatigue(Player& player, int value) {
        player.getStats().fatigue += player.scalePenalty(value);
    }

    void reduceFatigue(Player& player, int value) {
        player.getStats().fatigue -= player.scaleGain(value);
    }

    void reduceStress(Player& player, int value) {
        player.getStats().stress -= player.scaleGain(value);
    }

    void gainStress(Player& player, int value) {
        player.getStats().stress += player.scalePenalty(value);
    }

    void gainHumanity(Player& player, int value) {
        player.getStats().humanity += player.scaleGain(value);
    }

    void loseHumanity(Player& player, int value) {
        player.getStats().humanity -= player.scalePenalty(value);
    }

    void gainRomance(Player& player, int value) {
        player.getStats().romance += player.scaleGain(value);
    }

    void loseRomance(Player& player, int value) {
        player.getStats().romance -= player.scalePenalty(value);
    }

    void gainHunger(Player& player, int value) {
        player.getStats().hunger = std::min(
            GameConstants::MAX_HUNGER,
            player.getStats().hunger + player.scaleGain(value)
        );
    }

    void loseHunger(Player& player, int value) {
        player.getStats().hunger -= player.scalePenalty(value);
    }

    void loseHealth(Player& player, int value) {
        player.getStats().health -= player.scalePenalty(value);
    }

    void gainMoney(Player& player, int value) {
        player.getStats().money += player.scaleMoneyGain(value);
    }

    bool spendMoney(Player& player, int baseCost) {
        int cost = player.scaleCost(baseCost);

        if (player.getStats().money < cost) {
            return false;
        }

        player.getStats().money -= cost;
        return true;
    }

    int scoreToMark(int score) {
        if (score >= 85) return 5;
        if (score >= 70) return 4;
        if (score >= GameConstants::EXAM_PASS_THRESHOLD) return 3;
        return 2;
    }

    std::string scoreResultText(int score) {
        return "Балл: " + std::to_string(score) + "/100. Оценка: " + std::to_string(scoreToMark(score)) + ".";
    }

    void writeManualExamResult(Player& player, int examId, int score) {
        int oldScore = player.getGrade(examId);
        bool hadFailedGrade = oldScore > 0 && oldScore < GameConstants::EXAM_PASS_THRESHOLD;

        score = std::clamp(score, 0, 100);
        player.setGrade(examId, score);

        if (score >= GameConstants::EXAM_PASS_THRESHOLD) {
            if (hadFailedGrade) {
                player.removeDebt();
            }
        } else {
            if (!hadFailedGrade) {
                player.addDebt();
            }
        }
    }

    int randomInt(int minValue, int maxValue) {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<int> dis(minValue, maxValue);
        return dis(gen);
    }
}


Game::Game() {
    ConsoleUI::SetConsoleUTF8();
    // Load saved language preference
    if (!UIModeManager::isRussian()) {
        Lang::set(Lang::English);
    }
}

void Game::initNPCs() {
    alla = std::make_unique<Alla>();
    bulat = std::make_unique<Bulat>();
    semen = std::make_unique<Semen>();
    artem = std::make_unique<Artem>();
}

DifficultyLevel Game::selectDifficulty() {
    Player preview("Тимур");

    ConsoleUI::RenderScreen(
        "ВЫБОР СЛОЖНОСТИ",
        R"(Выбери режим перед началом игры.

1) Изи мод — показатели фармятся легче, штрафы мягкие, деньги тратятся легче.
2) Нормальный — нужен менеджмент ресурсов, но игра не душит за каждую ошибку.
3) Хард — проходится, но требует почти идеальной игры. Ошибки больно бьют.)",
        {
            "Изи мод",
            "Нормальный уровень",
            "Хард"
        },
        preview,
        "",
        "",
        "Настройка игры"
    );

    int choice = ConsoleUI::ReadInt("", 1, 3);

    DifficultyLevel difficulty = DifficultyLevel::Normal;

    if (choice == 1) {
        difficulty = DifficultyLevel::Easy;
    } else if (choice == 3) {
        difficulty = DifficultyLevel::Hard;
    }

    preview.setDifficulty(difficulty);

    ConsoleUI::RenderScreen(
        "СЛОЖНОСТЬ ВЫБРАНА",
        "Выбран режим: " + preview.getDifficultyName() + "\n\n" + preview.getDifficultyDescription(),
        {},
        preview,
        "",
        "",
        "Настройка игры"
    );
    ConsoleUI::WaitForEnter();

    return difficulty;
}

void Game::applyDifficultyStartSettings(DifficultyLevel difficulty) {
    Player& player = state.getPlayer();
    auto& stats = player.getStats();

    player.setDifficulty(difficulty);

    stats.money = DifficultyRules::StartingMoney(difficulty, GameConstants::START_MONEY);
    stats.stress = DifficultyRules::StartingStress(difficulty, GameConstants::START_STRESS);
    stats.fatigue = DifficultyRules::StartingFatigue(difficulty, GameConstants::START_FATIGUE);
    stats.intellect = DifficultyRules::StartingIntellect(difficulty, GameConstants::START_INTELLECT);

    if (difficulty == DifficultyLevel::Easy) {
        stats.energy = std::min(GameConstants::MAX_STAT, stats.energy + 10);
        stats.humanity = std::min(GameConstants::MAX_STAT, stats.humanity + 5);
        player.modifyRelation("Алла", 5);
        player.modifyRelation("Булат", 5);
        player.modifyRelation("Семён", 5);
        player.modifyRelation("Артём", 5);
        player.modifyRelation("Преподаватели", 5);
    } else if (difficulty == DifficultyLevel::Hard) {
        stats.energy = std::max(GameConstants::MIN_STAT, stats.energy - 10);
        stats.humanity = std::max(GameConstants::MIN_STAT, stats.humanity - 5);
        player.modifyRelation("Алла", -5);
        player.modifyRelation("Булат", -5);
        player.modifyRelation("Семён", -5);
        player.modifyRelation("Артём", -5);
        player.modifyRelation("Преподаватели", -5);
    }

    stats.clampAll();
}

void Game::initGame() {
    state = GameState();
    state.getPlayer() = Player("Тимур");

    DifficultyLevel difficulty = selectDifficulty();
    applyDifficultyStartSettings(difficulty);

    state.setGameStarted(true);
    state.setPhase(GamePhase::Playing);
    state.setCurrentDay(1);
    initNPCs();
    eventManager = RandomEventManager();
    achievements = AchievementSystem();
    journal.clear();
    encyclopedia = Encyclopedia();
    habits = HabitSystem();
    addJournalEntry("Дом", "Началась новая игра.", "story", 4);
}

void Game::run() {
    while (running) {
        switch (state.getPhase()) {
        case GamePhase::MainMenu:
            Menu::ShowMainMenu();

            switch (Menu::MainMenuChoice()) {
            case 1:
                initGame();
                runPrologue();
                break;

            case 2:
                initNPCs();
                loadGame();
                if (state.isGameStarted()) {
                    state.setPhase(GamePhase::Playing);
                }
                break;

            case 3:
                Menu::ShowControls();
                break;

            case 4:
                Menu::ShowSettings();
                break;

            case 5:
                DevMode::ShowDevMenu();
                break;

            case 6:
                Credits::ShowCreditsMain();
                break;

            case 0:
                running = false;
                break;
            }

            break;

        case GamePhase::Playing: {
            int day = state.getPlayer().getCurrentDay();

            // Утро дома: можно поесть после пробуждения.
            if (day >= 1 && day <= GameConstants::TOTAL_DAYS) {
                offerHomeMeal("УТРО ДОМА", false);
                applyDailySystems();
                checkGameOver();

                if (state.getPhase() != GamePhase::Playing) {
                    break;
                }
            }

            switch (day) {
            case 1:
                runDay1();
                break;
            case 2:
                runDay2();
                break;
            case 3:
                runDay3();
                break;
            case 4:
                runDay4();
                break;
            case 5:
                runDay5();
                break;
            case 6:
                runDay6();
                break;
            case 7:
                runDay7();
                break;
            case 8:
                runDay8();
                break;

            default: {
                auto ending = EndingSystem::EvaluateEnding(state.getPlayer());
                state.setGameOverReason(ending);
                state.setPhase(GamePhase::GameOver);
                break;
            }
            }

            checkGameOver();

            // Вечер дома: можно поесть перед сном.
            // На 8-й день не спрашиваем, потому что после него уже финал.
            if (state.getPhase() == GamePhase::Playing && day < GameConstants::TOTAL_DAYS) {
                offerHomeMeal("ВЕЧЕР ДОМА", true);
                applyDailySystems();
                checkGameOver();
            }

            if (state.getPhase() == GamePhase::Playing) {
                state.getPlayer().nextDay();
                applyDailySystems();
                saveGame();
            }

            break;
        }

        case GamePhase::GameOver: {
            Menu::ShowGameOverMenu(
                EndingSystem::GetEndingText(state.getGameOverReason())
            );

            int choice = Menu::MainMenuChoice();

            if (choice == 1) {
                initGame();
                runPrologue();
            } else if (choice == 2 || choice == 0) {
                state.setPhase(GamePhase::MainMenu);
            }

            break;
        }

        default:
            break;
        }
    }
}

void Game::applyDailySystems() {
    FatigueSystem::Update(state.getPlayer());
    HungerSystem::Update(state.getPlayer());
    DebuffSystem::Update(state.getPlayer());
    checkAchievements();
}

void Game::checkGameOver() {
    auto reason = EndingSystem::CheckEnding(state.getPlayer());
    if (reason != GameOverCondition::None) {
        state.setGameOverReason(reason);
        state.setPhase(GamePhase::GameOver);
    }
}

void Game::addJournalEntry(const std::string& location,
                           const std::string& description,
                           const std::string& category,
                           int importance) {
    journal.addEntry(
        state.getPlayer().getCurrentDay(),
        location,
        description,
        category,
        importance
    );
}

void Game::checkAchievements() {
    Player& player = state.getPlayer();
    const auto& stats = player.getStats();

    auto unlockAndNotify = [&](const std::string& id) {
        if (!achievements.unlock(id)) {
            return;
        }

        const Achievement* achievement = achievements.getAchievement(id);
        if (!achievement) {
            return;
        }

        journal.addEntry(
            player.getCurrentDay(),
            "Система",
            "Открыто достижение: " + achievement->title,
            "achievement",
            4
        );

        ConsoleUI::RenderScreen(
            "ДОСТИЖЕНИЕ",
            "Открыто новое достижение!\n\n" + achievement->title + "\n" + achievement->description,
            {},
            player,
            "",
            "",
            "Система"
        );
        ConsoleUI::WaitForEnter();
    };

    if (stats.money >= 5000) {
        unlockAndNotify("millionaire");
    }

    if (player.getCurrentDay() >= GameConstants::TOTAL_DAYS) {
        unlockAndNotify("survivor");
    }

    bool hasAnyExam = false;
    bool hasExcellentExam = false;
    bool allGoodExams = true;

    for (int examId = 1; examId <= 5; examId++) {
        int grade = player.getGrade(examId);

        if (grade > 0) {
            hasAnyExam = true;
        }

        if (grade >= 85) {
            hasExcellentExam = true;
        }

        if (grade < 70) {
            allGoodExams = false;
        }
    }

    if (hasAnyExam) {
        unlockAndNotify("first_exam");
    }

    if (hasExcellentExam) {
        unlockAndNotify("excellent");
    }

    if (allGoodExams) {
        unlockAndNotify("all_exams");
    }

    if (player.getRelation("Алла") >= 80 ||
        player.getRelation("Булат") >= 80 ||
        player.getRelation("Семён") >= 80 ||
        player.getRelation("Артём") >= 80) {
        unlockAndNotify("friend");
    }

    if (player.getRelation("Алла") >= 80 && stats.romance >= 60) {
        unlockAndNotify("romance");
    }
}

void Game::showAchievements() {
    std::ostringstream text;
    text << "Открыто: " << achievements.getUnlockedCount()
         << "/" << achievements.getTotalCount() << "\n\n";

    for (const Achievement* achievement : achievements.getAll()) {
        text << (achievement->unlocked ? "[+] " : "[-] ")
             << achievement->title << " — " << achievement->description << "\n";
    }

    ConsoleUI::RenderScreen(
        "ДОСТИЖЕНИЯ",
        text.str(),
        {},
        state.getPlayer(),
        "",
        "",
        "Дом"
    );
    ConsoleUI::WaitForEnter();
}

void Game::showJournal() {
    std::ostringstream text;
    auto recentEntries = journal.getRecent(12);

    if (recentEntries.empty()) {
        text << "Журнал пока пуст. События появятся по мере прохождения.";
    } else {
        for (const auto& entry : recentEntries) {
            text << entry.toString() << "\n";
        }
    }

    ConsoleUI::RenderScreen(
        "ЖУРНАЛ СОБЫТИЙ",
        text.str(),
        {},
        state.getPlayer(),
        "",
        "",
        "Дом"
    );
    ConsoleUI::WaitForEnter();
}

void Game::showEncyclopedia() {
    std::ostringstream text;
    auto names = encyclopedia.getAllNames();

    for (const auto& name : names) {
        const NPCLore* entry = encyclopedia.getEntry(name);
        if (!entry) {
            continue;
        }

        if (entry->discovered) {
            text << entry->formatEntry() << "\n\n";
        } else {
            text << "??? — запись ещё не открыта. Поговори с этим персонажем.\n\n";
        }
    }

    ConsoleUI::RenderScreen(
        "ЭНЦИКЛОПЕДИЯ NPC",
        text.str(),
        {},
        state.getPlayer(),
        "",
        "",
        "Дом"
    );
    ConsoleUI::WaitForEnter();
}

void Game::showHabits() {
    std::ostringstream text;

    for (const Habit* habit : habits.getAllHabits()) {
        text << (habit->isPositive ? "[+] " : "[-] ")
             << habit->title
             << " | серия: " << habit->streak
             << " | рекорд: " << habit->bestStreak
             << " | всего дней: " << habit->totalDays
             << "\n"
             << habit->description << "\n\n";
    }

    ConsoleUI::RenderScreen(
        "ПРИВЫЧКИ",
        text.str(),
        {},
        state.getPlayer(),
        "",
        "",
        "Дом"
    );
    ConsoleUI::WaitForEnter();
}


void Game::offerHomeMeal(const std::string& title, bool beforeSleep) {
    Player& player = state.getPlayer();
    auto& stats = player.getStats();

    player.setLocation(LocationID::Home);

    std::string text;

    if (beforeSleep) {
        text =
            "Ты вернулся домой перед сном.\n"
            "Сытость сейчас: " + std::to_string(stats.hunger) + "/100.\n"
            "Можно поесть за деньги, чтобы не проснуться голодным.";
    } else {
        text =
            "Ты проснулся дома.\n"
            "Сытость сейчас: " + std::to_string(stats.hunger) + "/100.\n"
            "Можно поесть перед тем, как идти по делам.";
    }

    int mealCost = scaledCost(player, GameConstants::EAT_MONEY_COST);
    int mealRestore = player.scaleGain(GameConstants::EAT_HUNGER_RESTORE);

    ConsoleUI::RenderScreen(
        title,
        text,
        {
            "Поесть дома (-" + std::to_string(mealCost) +
                " руб, +" + std::to_string(mealRestore) +
                " сытости)",
            "Не есть"
        },
        player,
        "",
        "",
        "Дом"
    );

    int choice = ConsoleUI::ReadInt();

    if (choice == 1) {
        if (stats.hunger >= GameConstants::MAX_HUNGER) {
            ConsoleUI::RenderScreen(
                "ЕДА",
                "Ты и так полностью сыт. Есть сейчас нет смысла.",
                {},
                player,
                "",
                "",
                "Дом"
            );
        } else if (stats.money < mealCost) {
            ConsoleUI::RenderScreen(
                "ЕДА",
                "Не хватает денег на еду.",
                {},
                player,
                "",
                "",
                "Дом"
            );
        } else {
            HungerSystem::Eat(player, GameConstants::EAT_MONEY_COST);

            ConsoleUI::RenderScreen(
                "ЕДА",
                "Ты поел дома.\n"
                "Сытость восстановлена до " + std::to_string(player.getStats().hunger) + "/100.\n"
                "Деньги: " + std::to_string(player.getStats().money) + " руб.",
                {},
                player,
                "",
                "",
                "Дом"
            );
        }

        ConsoleUI::WaitForEnter();
    }
}


void Game::saveGame() {
    // Serialize NPC memory into state
    std::string npcData;
    auto addNPCData = [&](const NPC& npc) {
        npcData += npc.serialize() + "---NPC_END---\n";
    };
    addNPCData(*alla);
    addNPCData(*bulat);
    addNPCData(*semen);
    addNPCData(*artem);
    state.setNPCMemoryData(npcData);
    state.setJournalData(journal.serialize());
    state.setEncyclopediaData(encyclopedia.serialize());
    state.setAchievementsData(achievements.serialize());
    state.setHabitsData(habits.serialize());
    SaveManager::SaveGame(state);
}

void Game::loadGame() {
    SaveManager::LoadGame(state);
    if (state.isGameStarted()) {
        state.setPhase(GamePhase::Playing);
        // Restore NPC memory
        std::string npcData = state.getNPCMemoryData();
        if (!npcData.empty()) {
            auto restoreNPC = [&](NPC& npc, const std::string& expectedName) {
                size_t pos = npcData.find("---NPC_END---");
                if (pos != std::string::npos) {
                    std::string block = npcData.substr(0, pos);
                    // Check if this block belongs to this NPC by finding the name
                    if (block.find(expectedName) != std::string::npos) {
                        npc.deserialize(block);
                    }
                    npcData = npcData.substr(pos + 13);
                }
            };
            restoreNPC(*alla, "Алла");
            restoreNPC(*bulat, "Булат");
            restoreNPC(*semen, "Семён");
            restoreNPC(*artem, "Артём");
        }

        if (!state.getJournalData().empty()) {
            journal.deserialize(state.getJournalData());
        }

        if (!state.getEncyclopediaData().empty()) {
            encyclopedia.deserialize(state.getEncyclopediaData());
        }

        if (!state.getAchievementsData().empty()) {
            achievements.deserialize(state.getAchievementsData());
        }

        if (!state.getHabitsData().empty()) {
            habits.deserialize(state.getHabitsData());
        }
    }
}

void Game::recordNPCChoice(const std::string& npcName, const std::string& choiceKey, int value) {
    NPC* npc = nullptr;
    if (npcName == "Алла") npc = alla.get();
    else if (npcName == "Булат") npc = bulat.get();
    else if (npcName == "Семён") npc = semen.get();
    else if (npcName == "Артём") npc = artem.get();
    if (!npc) return;
    npc->rememberEvent(choiceKey, value);

    // Auto-record detailed action based on choiceKey
    NPCAction action;
    action.type = choiceKey;
    action.day = state.getPlayer().getCurrentDay();
    action.influence = std::abs(value);
    action.location = locationToString(state.getPlayer().getLocation());
    action.fulfilledPromise = true;

    if (choiceKey == "helped_player") {
        action.category = "helpful";
        action.emotion = "благодарность";
        action.result = "помог игроку";
    } else if (choiceKey == "was_rude") {
        action.category = "rude";
        action.emotion = "обида";
        action.result = "огорчён";
    } else if (choiceKey == "compliment") {
        action.category = "romance";
        action.emotion = "приятно удивлена";
        action.result = "понравился комплимент";
    } else if (choiceKey == "refused_date") {
        action.category = "selfish";
        action.emotion = "разочарование";
        action.result = "отказ от свидания";
        action.fulfilledPromise = false;
    } else if (choiceKey == "shared_info") {
        action.category = "trust";
        action.emotion = "доверие";
        action.result = "поделился информацией";
    } else if (choiceKey == "promise_broken") {
        action.category = "selfish";
        action.emotion = "обманут";
        action.result = "нарушил обещание";
        action.fulfilledPromise = false;
    } else {
        action.category = "helpful";
        action.emotion = "нейтрально";
        action.result = "взаимодействие";
    }
    npc->recordAction(action);
}

// ==================== ПРОЛОГ ====================

void Game::runPrologue() {
    ConsoleUI::ClearScreen();
    int w = UIModeManager::screenW();
    std::cout << BOX_TL << std::string(w, BOX_H[0]) << BOX_TR "\n";
    std::cout << BOX_V << rpad("", w) << BOX_V "\n";
    std::string prologueTitle = "ПРОЛОГ";
    int lt = (w - static_cast<int>(visLen(prologueTitle + "  "))) / 2;
    std::cout << BOX_V << rpad(std::string(lt, ' ') + prologueTitle, w) << BOX_V "\n";
    std::cout << BOX_V << rpad("", w) << BOX_V "\n";
    std::cout << BOX_L << std::string(w, BOX_H[0]) << BOX_R "\n";
    std::cout << BOX_V << rpad("", w) << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Лето пролетело незаметно. Как будто только вчера", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("прозвенел последний звонок в школе, а сегодня ты уже", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("стоишь на пороге Уфимского университета науки и", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("технологий. В руках — потрёпанная папка с документами,", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("в голове — каша из ожиданий и страхов.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Первая сессия — это не просто экзамены.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Это проверка на прочность.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << rpad("", w) << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Твои друзья:", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("  Булат — лучший друг, весёлый и надёжный.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("  Алла — одногруппница, умная и красивая.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("  Семён — старшекурсник, циничный, но знающий.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("  Артём — замкнутый гений программирования.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << rpad("", w) << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Впереди 8 дней, которые определят твою судьбу.", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Сможешь ли ты выстоять?", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << rpad("", w) << BOX_V "\n";
    std::cout << BOX_V "  " << rpad("Твоя история начинается прямо сейчас...", w - 4) << "  " BOX_V "\n";
    std::cout << BOX_V << rpad("", w) << BOX_V "\n";
    std::cout << BOX_BL << std::string(w, BOX_H[0]) << BOX_BR "\n";
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 1 — ИСТОРИЯ ====================

void Game::runDay1() {
    ConsoleUI::PrintDayHeader(1, "История — сюжетный экзамен");

    Player& player = state.getPlayer();
    auto& stats = player.getStats();

    ConsoleUI::RenderScreen(
        "ПЕРВЫЙ ЭКЗАМЕН",
        "Первый день сессии. Экзамен по истории.\n"
        "Ты стоишь перед аудиторией и понимаешь, что билет может решить всё.\n"
        "Алла нервно листает конспект, Булат пытается шутить, но тоже переживает.",
        {
            "Поддержать Аллу перед экзаменом",
            "Повторить даты вместе с Булатом",
            "Молча зайти в аудиторию"
        },
        player,
        "",
        "",
        "Университет"
    );

    int choice = ConsoleUI::ReadInt();

    if (choice == 1) {
        ConsoleUI::RenderScreen(
            "АЛЛА",
            "Ты говоришь Алле, что она точно справится.\n"
            "Она улыбается: «Спасибо... Мне правда стало спокойнее».\n"
            "Ты тоже немного выдыхаешь.",
            {},
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла"
        );
        player.modifyRelation("Алла", 8);
        gainRomance(player, 3);
        reduceStress(player, 5);
        recordNPCChoice("Алла", "helped_player", 1);
    } else if (choice == 2) {
        ConsoleUI::RenderScreen(
            "БУЛАТ",
            "Вы с Булатом быстро повторяете даты.\n"
            "Он путает князей, зато неожиданно вспоминает пару важных событий.",
            {},
            player,
            ConsoleUI::GetBulatPortrait(),
            "Булат"
        );
        player.modifyRelation("Булат", 6);
        gainIntellect(player, 3);
    } else {
        ConsoleUI::RenderScreen(
            "АУДИТОРИЯ",
            "Ты молча заходишь в аудиторию.\n"
            "Никого не поддержал, зато сохранил концентрацию.",
            {},
            player
        );
        reduceStress(player, 2);
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ВЫБОР БИЛЕТА",
        "Преподаватель кладёт перед тобой три билета.\n"
        "Какой вытянуть?",
        {
            "Билет слева",
            "Билет посередине",
            "Билет справа"
        },
        player
    );

    int ticketChoice = ConsoleUI::ReadInt();
    int ticketModifier = 0;

    if (ticketChoice == 2) {
        ticketModifier = 10;
        ConsoleUI::RenderScreen(
            "БИЛЕТ",
            "Средний билет оказался относительно знакомым.\n"
            "Есть шанс нормально ответить.",
            {},
            player
        );
    } else {
        ticketModifier = -5;
        ConsoleUI::RenderScreen(
            "БИЛЕТ",
            "Билет попался неприятный.\n"
            "Некоторые вопросы ты видел только мельком.",
            {},
            player
        );
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ЧТО ДЕЛАТЬ?",
        "Билет перед тобой. В голове пустовато.\n"
        "Нужно быстро выбрать стратегию.",
        {
            "Попросить помощь у Аллы",
            "Ждать, пока преподаватель выйдет, и найти ответы",
            "Отвечать самостоятельно"
        },
        player
    );

    choice = ConsoleUI::ReadInt();

    int score = 0;
    bool resultReady = false;

    if (choice == 1) {
        int allaRelation = player.getRelation("Алла");

        if (allaRelation >= 50) {
            ConsoleUI::RenderScreen(
                "ПОМОЩЬ АЛЛЫ",
                "Алла незаметно пишет тебе план ответа на черновике.\n"
                "Теперь вопрос — как сдавать?",
                {
                    "Подождать ещё час и спокойно переписать ответ",
                    "Сдать сразу, пока не забыл"
                },
                player,
                ConsoleUI::GetAllaPortrait(),
                "Алла"
            );

            int allaChoice = ConsoleUI::ReadInt();
            if (allaChoice == 1) {
                score = 75 + ticketModifier;
                ConsoleUI::RenderScreen(
                    "ОТВЕТ",
                    "Ты выжидаешь, аккуратно переписываешь ответ и сдаёшь работу.\n"
                    "Преподаватель задаёт пару вопросов, но ты держишься нормально.",
                    {},
                    player
                );
                player.modifyRelation("Алла", 6);
                gainRomance(player, 2);
                recordNPCChoice("Алла", "shared_info", 1);
            } else {
                score = 35 + ticketModifier;
                ConsoleUI::RenderScreen(
                    "ПАЛЕВО",
                    "Ты сдаёшь слишком быстро. Преподаватель сразу понимает, что что-то не так.\n"
                    "Дополнительные вопросы добивают ситуацию.",
                    {},
                    player
                );
                player.modifyRelation("Преподаватели", -8);
                gainStress(player, 15);
                loseHumanity(player, 5);
            }
            resultReady = true;
        } else {
            ConsoleUI::RenderScreen(
                "АЛЛА НЕ ПОМОГЛА",
                "Алла тихо качает головой: «Извини, я сама еле держусь».\n"
                "Придётся отвечать самому.",
                {},
                player,
                ConsoleUI::GetAllaPortrait(),
                "Алла"
            );
            player.modifyRelation("Алла", -3);
            ConsoleUI::WaitForEnter();
        }
    }

    if (!resultReady && choice == 2) {
        int waitHours = 0;
        bool finished = false;

        while (!finished && waitHours < 4) {
            ConsoleUI::RenderScreen(
                "ОЖИДАНИЕ",
                "Преподаватель всё ещё сидит в аудитории.\n"
                "Прошло часов ожидания: " + std::to_string(waitHours) + ".",
                {
                    "Подождать ещё час",
                    "Выйти в туалет и попытаться найти помощь"
                },
                player
            );

            int waitChoice = ConsoleUI::ReadInt();
            if (waitChoice == 1) {
                waitHours++;
                gainFatigue(player, 5);
                gainStress(player, 4);

                if (waitHours >= 4) {
                    score = 25 + ticketModifier;
                    ConsoleUI::RenderScreen(
                        "ВРЕМЯ ВЫШЛО",
                        "Время экзамена заканчивается.\n"
                        "Ты идёшь сдавать почти пустой билет.",
                        {},
                        player
                    );
                    finished = true;
                }
            } else {
                ConsoleUI::RenderScreen(
                    "ТУАЛЕТ",
                    "В коридоре ты встречаешь знакомого студента.\n"
                    "Он тоже сдавал этот билет в прошлом году.",
                    {
                        "Попросить его быстро объяснить билет",
                        "Просто умыться и вернуться"
                    },
                    player
                );

                int toiletChoice = ConsoleUI::ReadInt();
                if (toiletChoice == 1) {
                    score = 75 + ticketModifier;
                    ConsoleUI::RenderScreen(
                        "ПОМОЩЬ",
                        "Знакомый быстро объясняет основу ответа.\n"
                        "Ты возвращаешься в аудиторию и сдаёшь уже увереннее.",
                        {},
                        player
                    );
                    loseHumanity(player, 3);
                    reduceStress(player, 5);
                } else {
                    score = 35 + ticketModifier;
                    ConsoleUI::RenderScreen(
                        "ПРОВАЛ",
                        "Холодная вода не добавила знаний.\n"
                        "Преподаватель вызывает тебя, а билет почти пустой.",
                        {},
                        player
                    );
                    gainStress(player, 10);
                }
                finished = true;
            }
        }

        resultReady = true;
    }

    if (!resultReady) {
        ConsoleUI::RenderScreen(
            "САМОСТОЯТЕЛЬНЫЙ ОТВЕТ",
            "Ты решаешь отвечать сам.\n"
            "Преподаватель задаёт пять коротких вопросов по датам.",
            {},
            player
        );
        ConsoleUI::WaitForEnter();

        struct HistoryYearQuestion {
            std::string text;
            int answer;
        };

        std::vector<HistoryYearQuestion> questions = {
            {"В каком году была Куликовская битва?", 1380},
            {"В каком году произошло Крещение Руси?", 988},
            {"В каком году началась Отечественная война?", 1812},
            {"В каком году произошла Октябрьская революция?", 1917},
            {"В каком году был первый полёт человека в космос?", 1961}
        };

        int correctAnswers = 0;
        int disputes = 0;

        for (size_t i = 0; i < questions.size(); ++i) {
            ConsoleUI::PrintHeader("ВОПРОС " + std::to_string(i + 1));
            std::cout << questions[i].text << "\n";
            int answer = ConsoleUI::ReadInt("Твой ответ (год): ", 1, 3000);

            if (answer == questions[i].answer) {
                std::cout << "Правильно!\n";
                correctAnswers++;
            } else {
                std::cout << "Неправильно. Правильный ответ: " << questions[i].answer << "\n";
                int dispute = ConsoleUI::ReadInt("Будешь спорить с преподавателем? 1 - да, 2 - нет: ", 1, 2);

                if (dispute == 1) {
                    std::cout << "Ты споришь, но преподаватель становится только злее.\n";
                    player.modifyRelation("Преподаватели", -5);
                    gainStress(player, 3);
                    disputes++;
                }
            }
            ConsoleUI::WaitForEnter();
        }

        if (correctAnswers >= 5) score = 95;
        else if (correctAnswers >= 4) score = 80;
        else if (correctAnswers >= 3) score = 60;
        else score = 35;

        score += ticketModifier;
        score -= disputes * 3;
    }

    score = std::clamp(score + player.getExamScoreModifier(), 0, 100);
    writeManualExamResult(player, 1, score);

    ConsoleUI::RenderScreen(
        "РЕЗУЛЬТАТ ИСТОРИИ",
        scoreResultText(score) + "\n\n" +
        (score >= GameConstants::EXAM_PASS_THRESHOLD
            ? "Экзамен сдан. Первый бой пережит."
            : "Экзамен провален. Появился долг."),
        {},
        player
    );

    if (score >= 85) {
        gainHumanity(player, 5);
        reduceStress(player, 10);
    } else if (score < GameConstants::EXAM_PASS_THRESHOLD) {
        gainStress(player, 15);
        gainFatigue(player, 10);
    }

    gainFatigue(player, 15);
    loseHunger(player, 8);
    stats.clampAll();

    ConsoleUI::WaitForEnter();
    eventManager.tryTriggerEvent(player, 1);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 1");
    ConsoleUI::PrintPlayerStats(player);
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 2 — ЯИМП ====================

void Game::runDay2() {
    ConsoleUI::PrintDayHeader(2, "Языки и методы программирования");

    Player& player = state.getPlayer();
    auto& stats = player.getStats();

    ConsoleUI::RenderScreen(
        "ЭКЗАМЕН ПО ЯИМП",
        "Второй день. Экзамен по ЯиМП.\n"
        "Преподаватель раскладывает десять билетов.\n"
        "Ходит слух, что один из них счастливый.",
        {},
        player,
        "",
        "",
        "Университет"
    );
    ConsoleUI::WaitForEnter();

    ConsoleUI::PrintHeader("ВЫБОР БИЛЕТА");
    std::cout << "Выбери номер билета от 1 до 10.\n";
    int ticket = ConsoleUI::ReadInt("Номер билета: ", 1, 10);

    int score = 0;
    const int luckyTicket = 7;

    if (ticket == luckyTicket) {
        score = 95;
        ConsoleUI::RenderScreen(
            "СЧАСТЛИВЫЙ БИЛЕТ",
            "Ты вытянул счастливый билет.\n"
            "Вопросы попались ровно те, что ты повторял.\n"
            "Преподаватель доволен ответом.",
            {},
            player
        );
        reduceStress(player, 15);
    } else {
        score = 75;
        ConsoleUI::RenderScreen(
            "ОБЫЧНЫЙ БИЛЕТ",
            "Билет не счастливый, но вопросы оказались терпимыми.\n"
            "Ты отвечаешь не идеально, но уверенно.",
            {},
            player
        );
        gainIntellect(player, 2);
    }

    score = std::clamp(score + player.getExamScoreModifier(), 0, 100);
    writeManualExamResult(player, 2, score);

    ConsoleUI::RenderScreen(
        "РЕЗУЛЬТАТ ЯИМП",
        scoreResultText(score) + "\n\n" +
        (score >= GameConstants::EXAM_PASS_THRESHOLD
            ? "Экзамен сдан. Можно выдохнуть."
            : "Даже этот билет не спас. Появился долг."),
        {},
        player
    );
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ПОСЛЕ ЭКЗАМЕНА",
        "После экзамена ты идёшь в сторону столовой.\n"
        "Навстречу выходит Артём. Сегодня он явно не в настроении.\n"
        "Он преграждает путь и смотрит на тебя исподлобья.",
        {
            "Убежать, пока конфликт не начался",
            "Попытаться постоять за себя",
            "Молча отдать часть денег"
        },
        player,
        ConsoleUI::GetArtemPortrait(),
        "Артём"
    );

    int fightChoice = ConsoleUI::ReadInt();

    if (fightChoice == 1) {
        ConsoleUI::RenderScreen(
            "ПОБЕГ",
            "Ты резко обходишь Артёма и почти бежишь к лестнице.\n"
            "Конфликта удалось избежать, но выглядело это унизительно.",
            {},
            player
        );
        gainFatigue(player, 25);
        gainStress(player, 15);
        loseHumanity(player, 5);
        recordNPCChoice("Артём", "was_rude", 1);
    } else if (fightChoice == 2) {
        ConsoleUI::RenderScreen(
            "КОНФЛИКТ",
            "Ты пытаешься ответить жёстко.\n"
            "Артём толкает тебя плечом, начинается короткая стычка.\n"
            "Ничего серьёзного, но настроение испорчено.",
            {},
            player
        );
        loseHealth(player, 15);
        gainFatigue(player, 15);
        gainStress(player, 20);
        player.modifyRelation("Артём", -15);
    } else {
        int moneyToGive = stats.money / 4;

        if (moneyToGive > 0) {
            stats.money -= moneyToGive;
            ConsoleUI::RenderScreen(
                "ДЕНЬГИ",
                "Ты молча отдаёшь " + std::to_string(moneyToGive) + " руб.\n"
                "Артём теряет интерес и уходит.",
                {},
                player
            );
            gainStress(player, 5);
        } else {
            ConsoleUI::RenderScreen(
                "ДЕНЕГ НЕТ",
                "У тебя нет денег. Артём злится и толкает тебя.\n"
                "Ситуация заканчивается неприятно.",
                {},
                player
            );
            loseHealth(player, 10);
            gainFatigue(player, 15);
            gainStress(player, 15);
        }
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "СТОЛОВАЯ",
        "После конфликта ты всё же добираешься до столовой.\n"
        "Горячий обед немного возвращает тебя к жизни.",
        {},
        player
    );
    gainHunger(player, 25);
    gainEnergy(player, 15);
    ConsoleUI::WaitForEnter();

    eventManager.tryTriggerEvent(player, 2);

    stats.clampAll();
    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 2");
    ConsoleUI::PrintPlayerStats(player);
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 3 — ПОДГОТОВКА ====================

void Game::runDay3() {
    ConsoleUI::PrintDayHeader(3, "Физкультура и 3D-моделирование");

    Player& player = state.getPlayer();
    auto& stats = player.getStats();

    ConsoleUI::RenderScreen(
        "ДЕНЬ БЕЗ ЭКЗАМЕНА",
        "Третий день. Экзаменов сегодня нет.\n"
        "В расписании физкультура и 3D-моделирование.\n"
        "Завтра дискретная математика, поэтому любое решение может помочь или навредить.",
        {},
        player
    );
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ПО ДОРОГЕ НА ФИЗКУЛЬТУРУ",
        "Навстречу попадается Семён.\n"
        "Семён: «Привет! После физры пойдёшь со мной обедать? Я угощаю».\n"
        "Отвечать надо сейчас.",
        {
            "Согласиться, но дать точный ответ после физкультуры",
            "Сразу отказаться — хочешь побыть один"
        },
        player,
        ConsoleUI::GetSemenPortrait(),
        "Семён"
    );

    int semenLunchChoice = ConsoleUI::ReadInt();

    if (semenLunchChoice == 1) {
        ConsoleUI::RenderScreen(
            "СЕМЁН",
            "Ты обещаешь дать ответ после физкультуры.\n"
            "Семён довольно кивает: «Хорошо, жду».\n"
            "Теперь главное — пережить пару.",
            {},
            player
        );
        player.modifyRelation("Семён", 3);
    } else {
        ConsoleUI::RenderScreen(
            "СЕМЁН",
            "Ты сразу отказываешься.\n"
            "Семён делает вид, что ему всё равно, но видно, что он разочарован.",
            {},
            player
        );
        player.modifyRelation("Семён", -5);
        recordNPCChoice("Семён", "was_rude", 1);
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ФИЗКУЛЬТУРА",
        "Пара начинается жёстко: бег, отжимания, подтягивания.\n"
        "После занятия ноги гудят, но голова стала свежее.",
        {},
        player
    );
    loseEnergy(player, 25);
    gainFatigue(player, 20);
    reduceStress(player, 8);
    player.advanceTime(90);
    ConsoleUI::WaitForEnter();

    if (semenLunchChoice == 1) {
        ConsoleUI::RenderScreen(
            "ОБЕДЕННЫЙ ПЕРЕРЫВ",
            "Семён подходит после физкультуры: «Ну что, идём?»",
            {
                "Пойти с Семёном в столовую",
                "Отказаться и поесть одному"
            },
            player,
            ConsoleUI::GetSemenPortrait(),
            "Семён"
        );

        int lunchChoice = ConsoleUI::ReadInt();
        if (lunchChoice == 1) {
            ConsoleUI::RenderScreen(
                "СТОЛОВАЯ С СЕМЁНОМ",
                "Вы идёте в столовую вместе. Семён угощает тебя обедом.\n"
                "За едой он рассказывает пару полезных историй о преподавателях.",
                {},
                player
            );
            gainHunger(player, 30);
            gainEnergy(player, 20);
            player.modifyRelation("Семён", 10);
            recordNPCChoice("Семён", "helped_player", 1);
        } else {
            ConsoleUI::RenderScreen(
                "ОТКАЗ",
                "Ты говоришь, что хочешь поесть один.\n"
                "Семён разочарованно уходит.",
                {},
                player
            );
            gainHunger(player, 20);
            gainEnergy(player, 12);
            player.modifyRelation("Семён", -8);
            recordNPCChoice("Семён", "was_rude", 1);
        }
        ConsoleUI::WaitForEnter();
    } else {
        ConsoleUI::RenderScreen(
            "ОБЕД ОДНОМУ",
            "Ты ешь один. Спокойно, но немного пусто.\n"
            "Зато никто не отвлекает от мыслей о дискретке.",
            {},
            player
        );
        gainHunger(player, 18);
        gainIntellect(player, 2);
        ConsoleUI::WaitForEnter();
    }

    ConsoleUI::RenderScreen(
        "3D-МОДЕЛИРОВАНИЕ",
        "На паре по 3D-моделированию ты замечаешь Аллу.\n"
        "Она сидит одна за компьютером. Можно подсесть и начать разговор.",
        {
            "Начать милый разговор",
            "Спросить только про учёбу"
        },
        player,
        ConsoleUI::GetAllaPortrait(),
        "Алла"
    );

    int allaChoice = ConsoleUI::ReadInt();
    if (allaChoice == 1) {
        ConsoleUI::RenderScreen(
            "АЛЛА",
            "Ты спрашиваешь, как она провела выходные, и делаешь лёгкий комплимент.\n"
            "Алла смущается, но явно рада разговору.",
            {},
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла"
        );
        player.modifyRelation("Алла", 12);
        gainEnergy(player, 8);
        gainRomance(player, 8);
        recordNPCChoice("Алла", "compliment", 1);
    } else {
        ConsoleUI::RenderScreen(
            "АЛЛА",
            "Ты спрашиваешь только про подготовку к экзаменам.\n"
            "Алла сухо отвечает и отворачивается к монитору.\n"
            "Кажется, она ждала не этого.",
            {},
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла"
        );
        player.modifyRelation("Алла", -10);
        gainStress(player, 5);
        recordNPCChoice("Алла", "was_rude", 1);
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ПОДГОТОВКА К ДИСКРЕТКЕ",
        "Вечером ты всё же садишься за дискретную математику.\n"
        "Не идеально, но основные темы становятся понятнее.",
        {},
        player
    );
    gainIntellect(player, 8);
    gainFatigue(player, 10);
    player.advanceTime(120);
    ConsoleUI::WaitForEnter();

    eventManager.tryTriggerEvent(player, 3);

    stats.clampAll();
    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 3");
    ConsoleUI::PrintPlayerStats(player);
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 4 — ДИСКРЕТНАЯ МАТЕМАТИКА ====================

void Game::runDay4() {
    ConsoleUI::PrintDayHeader(4, "Дискретная математика — сюжетный экзамен");

    Player& player = state.getPlayer();
    auto& stats = player.getStats();

    ConsoleUI::RenderScreen(
        "НОЧЬ ПЕРЕД ЭКЗАМЕНОМ",
        "Ночь перед дискретной математикой.\n"
        "Ты не можешь уснуть: графы, деревья, булевы функции — всё смешалось в голове.",
        {
            "Лечь спать и восстановиться",
            "Играть всю ночь, чтобы снять стресс"
        },
        player
    );

    int choice = ConsoleUI::ReadInt();
    if (choice == 1) {
        ConsoleUI::RenderScreen(
            "СОН",
            "Ты заставляешь себя лечь. Сон выходит тревожным, но организм благодарен.",
            {},
            player
        );
        reduceFatigue(player, 30);
        gainEnergy(player, 25);
    } else {
        ConsoleUI::RenderScreen(
            "ИГРЫ ВСЮ НОЧЬ",
            "Ты играешь почти до утра. Стресс отпустил, но тело просит пощады.",
            {},
            player
        );
        reduceStress(player, 25);
        gainFatigue(player, 35);
        loseEnergy(player, 20);
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ПЕРЕД ЭКЗАМЕНОМ",
        "Перед аудиторией ты замечаешь Булата.\n"
        "Он выглядит очень встревоженным:\n"
        "«Брат, если я завалю, меня точно добьют долги».\n"
        "Времени мало — можно помочь ему или заняться собой.",
        {
            "Помочь Булату повторить темы",
            "Не помогать и поискать Аллу"
        },
        player,
        ConsoleUI::GetBulatPortrait(),
        "Булат"
    );

    choice = ConsoleUI::ReadInt();
    bool helpedBulat = false;

    if (choice == 1) {
        helpedBulat = true;
        ConsoleUI::RenderScreen(
            "ПОМОЩЬ БУЛАТУ",
            "Вы вместе повторяете основные темы.\n"
            "Булат становится увереннее, но ты тратишь силы и время.",
            {},
            player,
            ConsoleUI::GetBulatPortrait(),
            "Булат"
        );
        player.modifyRelation("Булат", 15);
        gainFatigue(player, 15);
        gainIntellect(player, 2);
        recordNPCChoice("Булат", "helped_player", 1);
    } else {
        ConsoleUI::RenderScreen(
            "АЛЛА",
            "Ты оставляешь Булата и ищешь Аллу.\n"
            "Она рада, что ты подошёл, но Булат остаётся один со своей паникой.",
            {},
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла"
        );
        player.modifyRelation("Алла", 8);
        gainRomance(player, 5);
        player.modifyRelation("Булат", -10);
        recordNPCChoice("Булат", "was_rude", 1);
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ШПАРГАЛКА ОТ СЕМЁНА",
        "Перед входом к тебе подходит Семён и протягивает маленький листок:\n"
        "«Там формулы по графам. Пользоваться или нет — решай сам».\n"
        "Это может спасти, но честностью тут не пахнет.",
        {
            "Взять шпаргалку",
            "Отказаться",
            "Попросить Семёна быстро объяснить сложные темы"
        },
        player,
        ConsoleUI::GetSemenPortrait(),
        "Семён"
    );

    choice = ConsoleUI::ReadInt();
    bool tookCheatSheet = false;

    if (choice == 1) {
        tookCheatSheet = true;
        player.setFlag("took_cheat_sheet", true);
        player.modifyRelation("Семён", 5);
        loseHumanity(player, 5);
        ConsoleUI::RenderScreen("ШПАРГАЛКА", "Ты прячешь шпаргалку в карман.", {}, player);
    } else if (choice == 2) {
        player.setFlag("took_cheat_sheet", false);
        player.modifyRelation("Семён", -2);
        gainHumanity(player, 5);
        ConsoleUI::RenderScreen("ОТКАЗ", "Ты отказываешься. Семён пожимает плечами: «Дело твоё».", {}, player);
    } else {
        player.setFlag("took_cheat_sheet", false);
        gainIntellect(player, 5);
        player.modifyRelation("Семён", 3);
        recordNPCChoice("Семён", "helped_player", 1);
        ConsoleUI::RenderScreen("ОБЪЯСНЕНИЕ", "Семён быстро объясняет пару важных моментов. Стало понятнее.", {}, player);
    }
    ConsoleUI::WaitForEnter();

    bool hardTicket = randomInt(0, 1) == 1;
    std::string ticketText = hardTicket
        ? "Ты тянешь билет. Попался сложный билет. Часть заданий выглядит как проклятье."
        : "Ты тянешь билет. Попался простой билет. Есть шанс нормально вывезти.";

    ConsoleUI::RenderScreen(
        "БИЛЕТ",
        ticketText,
        {
            "Решать самостоятельно",
            "Попросить помощь у Булата"
        },
        player
    );

    choice = ConsoleUI::ReadInt();
    int score = 0;

    if (!hardTicket) {
        if (choice == 1) {
            score = stats.fatigue <= 50 ? 75 : 60;
            ConsoleUI::RenderScreen(
                "РЕШЕНИЕ",
                stats.fatigue <= 50
                    ? "Усталость терпимая. Ты спокойно решаешь задания."
                    : "Усталость мешает. Ты допускаешь ошибки, но проходной результат вытягиваешь.",
                {},
                player
            );
        } else {
            int bulatRelation = player.getRelation("Булат");
            if (helpedBulat || bulatRelation >= 60) {
                score = 80;
                ConsoleUI::RenderScreen("ПОМОЩЬ БУЛАТА", "Булат помогает тебе с задачей. Вы оба вывозите.", {}, player);
                player.modifyRelation("Булат", 5);
            } else {
                score = stats.fatigue <= 50 ? 60 : 35;
                ConsoleUI::RenderScreen("БУЛАТ ОТКАЗАЛ", "Булат обижен и не помогает. Приходится выкручиваться самому.", {}, player);
                player.modifyRelation("Булат", -5);
            }
        }
    } else {
        if (choice == 1) {
            score = stats.fatigue <= 50 ? 60 : 35;
            ConsoleUI::RenderScreen(
                "СЛОЖНЫЙ БИЛЕТ",
                stats.fatigue <= 50
                    ? "Ты вспоминаешь часть материала и набираешь проходной минимум."
                    : "Усталость добивает. Сложный билет не прощает ошибок.",
                {},
                player
            );
        } else {
            int bulatRelation = player.getRelation("Булат");
            if (helpedBulat || bulatRelation >= 60) {
                score = 60;
                ConsoleUI::RenderScreen("ПОМОЩЬ БУЛАТА", "Булат помогает разобраться хотя бы с частью заданий.", {}, player);
                player.modifyRelation("Булат", 3);
            } else {
                score = 35;
                ConsoleUI::RenderScreen("ПРОВАЛ", "Булат не помогает. Сложный билет остаётся почти пустым.", {}, player);
                player.modifyRelation("Булат", -5);
            }
        }
    }

    if (tookCheatSheet && score < GameConstants::EXAM_PASS_THRESHOLD) {
        int oldScore = score;
        score = std::min(100, score + 10);
        ConsoleUI::RenderScreen(
            "ШПАРГАЛКА",
            "Шпаргалка помогла подсмотреть пару формул.\n"
            "Результат повышен на 10 баллов.",
            {},
            player
        );

        if (oldScore < GameConstants::EXAM_PASS_THRESHOLD &&
            score >= GameConstants::EXAM_PASS_THRESHOLD) {
            reduceStress(player, 10);
        }
        ConsoleUI::WaitForEnter();
    }

    score = std::clamp(score + player.getExamScoreModifier(), 0, 100);
    writeManualExamResult(player, 3, score);

    ConsoleUI::RenderScreen(
        "РЕЗУЛЬТАТ ДИСКРЕТКИ",
        scoreResultText(score) + "\n\n" +
        (score >= GameConstants::EXAM_PASS_THRESHOLD
            ? "Дискретная математика сдана."
            : "Дискретка провалена. Появился долг."),
        {},
        player
    );

    if (score >= 70) {
        reduceStress(player, 15);
        gainRomance(player, 3);
    } else if (score < GameConstants::EXAM_PASS_THRESHOLD) {
        gainStress(player, 25);
        loseRomance(player, 5);
    } else {
        gainStress(player, 5);
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen(
        "ПОСЛЕ ЭКЗАМЕНА",
        "Экзамен закончен. Ты чувствуешь усталость.\n"
        "В столовой удаётся немного восстановиться.",
        {},
        player
    );
    gainFatigue(player, 20);
    loseEnergy(player, 15);
    gainHunger(player, 25);
    gainEnergy(player, 15);
    gainIntellect(player, 5);
    ConsoleUI::WaitForEnter();

    eventManager.tryTriggerEvent(player, 4);

    stats.clampAll();
    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 4");
    ConsoleUI::PrintPlayerStats(player);
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 5 — МАТАНАЛИЗ ====================

void Game::runDay5() {
    ConsoleUI::PrintDayHeader(5, "Математический анализ");

    {
        ConsoleUI::RenderScreen("УТРО ПЕРЕД МАТАНОМ",
            "Самый страшный экзамен — матанализ.\n"
            "Алла подходит к тебе с обеспокоенным лицом:\n"
            "«Привет... Ты как? Я всю ночь не спала,\n"
            "всё повторяла ряды и интегралы.»",
            {"Подбодрить Аллу: «Ты умница, всё сдашь!»",
             "Сказать, что сам в панике и ничего не знаешь",
             "Предложить повторить вместе перед экзаменом",
             "Сказать, что купил ответы у Семёна"},
            state.getPlayer(), ConsoleUI::GetAllaPortrait(), "Алла");
    }

    int choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1:
        ConsoleUI::RenderScreen("АЛЛА", "Алла улыбается: «Спасибо, мне правда стало легче.\nТы очень поддерживаешь меня.»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", 10);
        gainRomance(state.getPlayer(), 5);
        recordNPCChoice("Алла", "helped_player", 1);
        break;
    case 2:
        ConsoleUI::RenderScreen("АЛЛА", "Алла вздыхает: «Мы оба в одной лодке... Ну, поплыли.»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", 2);
        break;
    case 3:
        ConsoleUI::RenderScreen("АЛЛА", "Вы садитесь на подоконник и полчаса повторяете интегралы.\nАлла благодарна за компанию.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", 12);
        gainRomance(state.getPlayer(), 5);
        gainIntellect(state.getPlayer(), 3);
        break;
    case 4: {
        std::string cheatText =
            "Алла удивлённо поднимает брови:\n"
            "«Ты купил ответы? Ну... надеюсь, тебя не поймают.\n"
            "Я буду сдавать честно.»";
        if (state.getPlayer().getStats().humanity > 50) {
            cheatText += "\n\nТы чувствуешь укол совести.";
        }
        ConsoleUI::RenderScreen("АЛЛА", cheatText, {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", -3);
        loseHumanity(state.getPlayer(), 10);
        ConsoleUI::WaitForEnter();

        // Возможность купить ответы
        int answersCost = scaledCost(state.getPlayer(), 300);

        ConsoleUI::RenderScreen("ПОКУПКА ОТВЕТОВ",
            "Ты находишь Семёна. Он продаёт ответы за " + std::to_string(answersCost) + " руб.\n"
            "Купишь?",
            {"Купить ответы за " + std::to_string(answersCost) + " руб",
             "Передумать и надеяться на свои силы"},
            state.getPlayer());

        choice = ConsoleUI::ReadInt();

        if (choice == 1 && state.getPlayer().getStats().money >= answersCost) {
            spendMoney(state.getPlayer(), 300);
            state.getPlayer().setFlag("bought_answers", true);
            ConsoleUI::RenderScreen("ПОКУПКА", "Ответы у тебя. Осталось только списать.", {}, state.getPlayer());
        } else {
            state.getPlayer().setFlag("bought_answers", false);
            ConsoleUI::RenderScreen("ПОКУПКА", "Ты решаешь положиться на свои знания.", {}, state.getPlayer());
        }
        ConsoleUI::WaitForEnter();
        break;
    }
    }
    ConsoleUI::WaitForEnter();

    // Экзамен по матанализу
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО МАТАНАЛИЗУ");
    CalculusExam calculusExam;
    int score = calculusExam.runExam(state.getPlayer());

    if (state.getPlayer().hasFlag("bought_answers") && score < GameConstants::EXAM_PASS_THRESHOLD) {
        int oldScore = score;

        ConsoleUI::RenderScreen("ОТВЕТЫ",
            "Купленные ответы помогли! +15 баллов.",
            {}, state.getPlayer());

        score = std::min(100, score + 15);
        state.getPlayer().setGrade(4, score);

        if (oldScore < GameConstants::EXAM_PASS_THRESHOLD &&
            score >= GameConstants::EXAM_PASS_THRESHOLD) {
            state.getPlayer().removeDebt();
        }

        ConsoleUI::WaitForEnter();
    }

    ConsoleUI::WaitForEnter();

    // После экзамена — Булат зовёт отметить
    {
        ConsoleUI::RenderScreen("ПОСЛЕ МАТАНА",
            "После изнурительного экзамена к тебе подходит Булат:\n"
            "«Фух... Это было жёстко. Надо отметить,\n"
            "что мы выжили! Пошли в бар?»",
            {"Пойти с Булатом — заслужили отдых",
             "Отказаться — нужно готовиться к следующему",
             "Пойти, но ненадолго"},
            state.getPlayer(), ConsoleUI::GetBulatPortrait(), "Булат");
    }

    choice = ConsoleUI::ReadInt("", 1, 3);

    if (choice == 1) {
        int barCost = scaledCost(state.getPlayer(), 200);

        if (state.getPlayer().getStats().money >= barCost) {
            ConsoleUI::RenderScreen("БАР", "Вы идёте в бар, берёте по пиву.\nОбсуждаете экзамены, смеётесь над преподавателями.\nНастроение улучшается!",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Булат", 8);
            reduceStress(state.getPlayer(), 15);
            gainFatigue(state.getPlayer(), 10);
            spendMoney(state.getPlayer(), 200);
        } else {
            ConsoleUI::RenderScreen("НЕТ ДЕНЕГ",
                "Ты проверяешь карманы и понимаешь, что на бар не хватает денег.\n"
                "Булат усмехается: «Ладно, брат, тогда просто прогуляемся. Главное — живы после матана».\n"
                "Деньги не списаны.",
                {}, state.getPlayer(), ConsoleUI::GetBulatPortrait(), "Булат");
            state.getPlayer().modifyRelation("Булат", 2);
            reduceStress(state.getPlayer(), 5);
        }
    } else if (choice == 3) {
        ConsoleUI::RenderScreen("ПРОГУЛКА", "Вы гуляете полчаса, обсуждаете планы.\nБулат понимает, что тебе нужно готовиться.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 5);
        reduceStress(state.getPlayer(), 5);
    } else {
        ConsoleUI::RenderScreen("ОТКАЗ", "Булат понимающе кивает: «Удачи с подготовкой, брат!»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", -2);
    }
    ConsoleUI::WaitForEnter();

    eventManager.tryTriggerEvent(state.getPlayer(), 5);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 5");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 6 — ВЫХОДНОЙ ====================

void Game::runDay6DatePreparation() {
    Player& player = state.getPlayer();

    bool hasFlowers = player.hasFlag("has_flowers");
    bool hasCandies = player.hasFlag("has_candies");

    int flowersCost = scaledCost(player, 300);
    int candiesCost = scaledCost(player, 250);
    int bothCost = scaledCost(player, 550);

    std::string info =
        "Завтра может быть свидание с Аллой.\n"
        "Можно заранее купить подарок.\n\n"
        "Уже куплено:\n"
        "- Цветы: " + std::string(hasFlowers ? "да" : "нет") + "\n" +
        "- Конфеты: " + std::string(hasCandies ? "да" : "нет") + "\n\n" +
        "Деньги: " + std::to_string(player.getStats().money) + " руб.";

    ConsoleUI::RenderScreen("ПОДГОТОВКА К СВИДАНИЮ",
        info,
        {"Купить цветы (" + std::to_string(flowersCost) + " руб.)",
         "Купить конфеты (" + std::to_string(candiesCost) + " руб.)",
         "Купить цветы и конфеты (" + std::to_string(bothCost) + " руб.)",
         "Ничего не покупать"},
        player,
        ConsoleUI::GetAllaPortrait(),
        "Алла");

    int choice = ConsoleUI::ReadInt("", 1, 4);

    if (choice == 1) {
        if (hasFlowers) {
            ConsoleUI::RenderScreen("ЦВЕТЫ", "У тебя уже есть цветы. Второй букет сейчас не нужен.", {}, player);
        } else if (spendMoney(player, 300)) {
            player.setFlag("has_flowers", true);
            ConsoleUI::RenderScreen("ЦВЕТЫ", "Ты купил красивый букет. Алле должно понравиться.", {}, player);
        } else {
            ConsoleUI::RenderScreen("НЕТ ДЕНЕГ", "На цветы не хватает денег. Покупка не совершена.", {}, player);
        }
    } else if (choice == 2) {
        if (hasCandies) {
            ConsoleUI::RenderScreen("КОНФЕТЫ", "У тебя уже есть конфеты.", {}, player);
        } else if (spendMoney(player, 250)) {
            player.setFlag("has_candies", true);
            ConsoleUI::RenderScreen("КОНФЕТЫ", "Ты купил коробку конфет. Нормальный безопасный подарок.", {}, player);
        } else {
            ConsoleUI::RenderScreen("НЕТ ДЕНЕГ", "На конфеты не хватает денег. Покупка не совершена.", {}, player);
        }
    } else if (choice == 3) {
        if (hasFlowers && hasCandies) {
            ConsoleUI::RenderScreen("ПОДАРКИ", "У тебя уже есть и цветы, и конфеты.", {}, player);
        } else if (spendMoney(player, 550)) {
            player.setFlag("has_flowers", true);
            player.setFlag("has_candies", true);
            ConsoleUI::RenderScreen("ПОДАРКИ", "Ты купил и цветы, и конфеты. Подготовился по полной.", {}, player);
        } else {
            ConsoleUI::RenderScreen("НЕТ ДЕНЕГ", "На полный набор подарков не хватает денег. Покупка не совершена.", {}, player);
        }
    } else {
        ConsoleUI::RenderScreen("БЕЗ ПОКУПОК", "Ты решил не тратить деньги заранее.", {}, player);
    }

    player.getStats().clampAll();
    ConsoleUI::WaitForEnter();
}

void Game::runDay6() {
    ConsoleUI::PrintDayHeader(6, "Выходной");

    Player& player = state.getPlayer();
    Stats& stats = player.getStats();

    player.setFlag("day7_available", true);
    player.setFlag("called_alla_day6", false);

    bool datePossible = player.hasFlag("date_with_alla_scheduled") ||
                        (player.getRelation("Алла") >= 60 && stats.romance >= 30) ||
                        player.hasFlag("invited_alla_to_walk");

    if (datePossible) {
        ConsoleUI::RenderScreen("УТРО ВЫХОДНОГО",
            "Сегодня выходной, а завтра может быть важная встреча с Аллой.\n"
            "Можно подготовиться заранее: купить цветы или конфеты.",
            {"Подготовиться к свиданию",
             "Не тратить деньги сейчас"},
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла");

        int prepChoice = ConsoleUI::ReadInt("", 1, 2);
        if (prepChoice == 1) {
            runDay6DatePreparation();
        }
    }

    bool continueDay = true;
    bool hasDoneAction = false;

    while (continueDay) {
        ConsoleUI::RenderScreen("ВЫХОДНОЙ",
            "Наконец-то выходной. Экзаменов сегодня нет.\n"
            "Можно отдохнуть, заработать денег или укрепить отношения.",
            {"Остаться дома и нормально отдохнуть",
             "Пойти работать",
             "Пойти гулять по городу",
             "Встретиться с друзьями",
             "Позвонить Алле",
             hasDoneAction ? "Закончить день" : "Закончить день (сначала сделай хоть что-нибудь)"},
            player);

        int choice = ConsoleUI::ReadInt("", 1, 6);

        if (choice == 6) {
            if (!hasDoneAction) {
                ConsoleUI::RenderScreen("РАНО",
                    "Ты только проснулся. Сделай хотя бы одно действие, а потом заканчивай день.",
                    {}, player);
                ConsoleUI::WaitForEnter();
                continue;
            }
            continueDay = false;
            break;
        }

        switch (choice) {
        case 1:
            ConsoleUI::RenderScreen("ДОМАШНИЙ ОТДЫХ",
                "Ты спишь до обеда, ешь нормальную еду, смотришь сериал и немного читаешь.\n"
                "Организм впервые за долгое время перестаёт ненавидеть тебя.",
                {}, player);
            gainEnergy(player, 40);
            reduceFatigue(player, 30);
            gainHunger(player, 15);
            reduceStress(player, 20);
            gainIntellect(player, 3);
            player.advanceTime(360);
            hasDoneAction = true;
            break;

        case 2:
            ConsoleUI::RenderScreen("РАБОТА",
                "Ты идёшь на подработку. День тяжёлый, зато деньги появляются.",
                {"Работать бариста",
                 "Работать курьером — больше денег, больше усталости"},
                player);
            choice = ConsoleUI::ReadInt("", 1, 2);
            if (choice == 1) {
                gainMoney(player, 500);
                loseEnergy(player, 20);
                gainFatigue(player, 20);
                gainStress(player, 8);
                loseHunger(player, 15);
            } else {
                gainMoney(player, 800);
                loseEnergy(player, 30);
                gainFatigue(player, 30);
                gainStress(player, 15);
                loseHunger(player, 20);
            }
            player.advanceTime(420);
            hasDoneAction = true;
            break;

        case 3:
            ConsoleUI::RenderScreen("ПРОГУЛКА",
                "Ты выходишь пройтись по городу. Свежий воздух немного чинит голову.",
                {"Зайти в цветочный магазин",
                 "Зайти в продуктовый",
                 "Просто погулять"},
                player);
            choice = ConsoleUI::ReadInt("", 1, 3);
            if (choice == 1) {
                if (spendMoney(player, 300)) {
                    player.setFlag("has_flowers", true);
                    ConsoleUI::RenderScreen("ЦВЕТЫ", "Ты купил цветы. Этот флаг пригодится на свидании.", {}, player);
                } else {
                    ConsoleUI::RenderScreen("НЕТ ДЕНЕГ", "На цветы не хватает денег.", {}, player);
                }
            } else if (choice == 2) {
                if (spendMoney(player, 250)) {
                    player.setFlag("has_candies", true);
                    ConsoleUI::RenderScreen("КОНФЕТЫ", "Ты купил конфеты. Теперь их можно подарить Алле.", {}, player);
                } else {
                    ConsoleUI::RenderScreen("НЕТ ДЕНЕГ", "На конфеты не хватает денег.", {}, player);
                }
            } else {
                reduceStress(player, 15);
                gainEnergy(player, 5);
                ConsoleUI::RenderScreen("ПРОГУЛКА", "Ты просто гуляешь без цели. Стало спокойнее.", {}, player);
            }
            player.advanceTime(120);
            hasDoneAction = true;
            break;

        case 4:
            ConsoleUI::RenderScreen("ДРУЗЬЯ",
                "Ты встречаешься с Булатом и Семёном. Булат предлагает настолки.",
                {"Монополия — весело и шумно",
                 "Шахматы с Семёном",
                 "Дурак с Булатом"},
                player);
            choice = ConsoleUI::ReadInt("", 1, 3);
            if (choice == 1) {
                state.getPlayer().modifyRelation("Булат", 5);
                state.getPlayer().modifyRelation("Семён", 5);
                reduceStress(player, 15);
            } else if (choice == 2) {
                state.getPlayer().modifyRelation("Семён", 8);
                gainIntellect(player, 4);
                reduceStress(player, 8);
            } else {
                ConsoleUI::RenderScreen("ДУРАК",
                    "Булат предлагает сыграть на 200 рублей.",
                    {"Играть на деньги",
                     "Играть просто так"},
                    player);
                int moneyGameChoice = ConsoleUI::ReadInt("", 1, 2);
                if (moneyGameChoice == 1 && player.getStats().money >= scaledCost(player, 200)) {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, 1);
                    if (dis(gen) == 0) {
                        gainMoney(player, 200);
                        state.getPlayer().modifyRelation("Булат", 6);
                        reduceStress(player, 10);
                        ConsoleUI::RenderScreen("ПОБЕДА", "Ты выиграл партию и забрал деньги.", {}, player);
                    } else {
                        spendMoney(player, 200);
                        state.getPlayer().modifyRelation("Булат", 3);
                        gainStress(player, 10);
                        ConsoleUI::RenderScreen("ПРОИГРЫШ", "Ты проиграл 200 рублей. Обидно, но бывает.", {}, player);
                    }
                } else {
                    state.getPlayer().modifyRelation("Булат", 4);
                    state.getPlayer().modifyRelation("Семён", 4);
                    reduceStress(player, 12);
                    ConsoleUI::RenderScreen("ПРОСТО ИГРА", "Вы играете без денег. Зато без лишней боли.", {}, player);
                }
            }
            player.advanceTime(240);
            hasDoneAction = true;
            break;

        case 5:
            if (player.hasFlag("called_alla_day6")) {
                ConsoleUI::RenderScreen("ЗВОНОК АЛЛЕ",
                    "Ты уже звонил Алле сегодня. Второй звонок будет выглядеть странно.",
                    {}, player);
                break;
            }

            if (player.hasFlag("date_with_alla_scheduled")) {
                ConsoleUI::RenderScreen("ЗВОНОК АЛЛЕ",
                    "Алла отвечает быстро.\n"
                    "«Я помню про завтра. Уже жду.»\n"
                    "После разговора становится спокойнее.",
                    {}, player,
                    ConsoleUI::GetAllaPortrait(),
                    "Алла");
                state.getPlayer().modifyRelation("Алла", 3);
                gainRomance(player, 2);
                reduceStress(player, 5);
            } else if (player.getRelation("Алла") >= 60) {
                ConsoleUI::RenderScreen("ЗВОНОК АЛЛЕ",
                    "Ты набираешь Аллу. Она отвечает радостно.\n"
                    "Можно просто поболтать или договориться о встрече завтра.",
                    {"Просто поговорить",
                     "Позвать Аллу на свидание завтра"},
                    player,
                    ConsoleUI::GetAllaPortrait(),
                    "Алла");
                int callChoice = ConsoleUI::ReadInt("", 1, 2);
                if (callChoice == 1) {
                    state.getPlayer().modifyRelation("Алла", 5);
                    gainRomance(player, 3);
                    reduceStress(player, 10);
                    recordNPCChoice("Алла", "called_day6", 1);
                } else {
                    state.getPlayer().modifyRelation("Алла", 8);
                    gainRomance(player, 8);
                    player.setFlag("date_with_alla_scheduled", true);
                    player.setFlag("invited_alla_to_walk", true);
                    recordNPCChoice("Алла", "date_scheduled", 1);
                    ConsoleUI::RenderScreen("СВИДАНИЕ",
                        "Алла немного смущается, но соглашается.\n"
                        "Завтра у тебя свидание.",
                        {}, player,
                        ConsoleUI::GetAllaPortrait(),
                        "Алла");
                }
            } else if (player.getRelation("Алла") >= 40) {
                ConsoleUI::RenderScreen("ЗВОНОК АЛЛЕ",
                    "Алла отвечает спокойно. Вы немного говорите об учёбе и жизни.\n"
                    "Пока это не свидание, но контакт стал лучше.",
                    {}, player,
                    ConsoleUI::GetAllaPortrait(),
                    "Алла");
                state.getPlayer().modifyRelation("Алла", 4);
                gainRomance(player, 2);
                reduceStress(player, 5);
            } else {
                ConsoleUI::RenderScreen("ЗВОНОК АЛЛЕ",
                    "Алла не берёт трубку. Похоже, отношения слишком холодные.",
                    {}, player);
                state.getPlayer().modifyRelation("Алла", -3);
                recordNPCChoice("Алла", "ignored_call", 1);
            }

            player.setFlag("called_alla_day6", true);
            player.advanceTime(60);
            hasDoneAction = true;
            break;
        }

        player.getStats().clampAll();
        checkGameOver();
        if (state.getPhase() != GamePhase::Playing) return;
        ConsoleUI::WaitForEnter();
    }

    eventManager.tryTriggerEvent(player, 6);

    ConsoleUI::RenderScreen("ВЕЧЕР",
        "Вечер подходит к концу. Ты ужинаешь и ложишься спать.\n"
        "Завтра будет важный день.",
        {}, player);
    gainHunger(player, 15);
    gainEnergy(player, 10);
    reduceStress(player, 5);
    player.getStats().clampAll();

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 6");
    ConsoleUI::PrintPlayerStats(player);
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 7 — СВИДАНИЕ ИЛИ ПЕРЕСДАЧА ====================

void Game::runDateWithAlla() {
    Player& player = state.getPlayer();
    bool hasFlowers = player.hasFlag("has_flowers");
    bool hasCandies = player.hasFlag("has_candies");

    ConsoleUI::RenderScreen("СВИДАНИЕ С АЛЛОЙ",
        "Ты встречаешь Аллу возле университета.\n"
        "Она улыбается, но внимательно смотрит на тебя.\n"
        "Подарки могут помочь, но разговоры важнее.",
        {}, player,
        ConsoleUI::GetAllaPortrait(),
        "Алла");
    ConsoleUI::WaitForEnter();

    if (hasFlowers || hasCandies) {
        std::vector<std::string> giftChoices;
        if (hasFlowers) giftChoices.push_back("Подарить цветы");
        if (hasCandies) giftChoices.push_back("Подарить конфеты");
        if (hasFlowers && hasCandies) giftChoices.push_back("Подарить и цветы, и конфеты");
        giftChoices.push_back("Пока ничего не дарить");

        ConsoleUI::RenderScreen("ПОДАРОК",
            "Что подаришь Алле в начале встречи?",
            giftChoices,
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла");

        int giftChoice = ConsoleUI::ReadInt("", 1, static_cast<int>(giftChoices.size()));
        std::string selected = giftChoices[giftChoice - 1];

        if (selected == "Подарить цветы") {
            ConsoleUI::RenderScreen("ЦВЕТЫ",
                "Ты даришь Алле цветы. Она улыбается и явно тронута.",
                {}, player,
                ConsoleUI::GetAllaPortrait(),
                "Алла");
            state.getPlayer().modifyRelation("Алла", 10);
            gainRomance(player, 10);
            player.setFlag("has_flowers", false);
            recordNPCChoice("Алла", "gave_flowers_to_alla", 1);
        } else if (selected == "Подарить конфеты") {
            ConsoleUI::RenderScreen("КОНФЕТЫ",
                "Ты даришь Алле конфеты. Она смеётся: «Сладкое после сессии — это лечение.»",
                {}, player,
                ConsoleUI::GetAllaPortrait(),
                "Алла");
            state.getPlayer().modifyRelation("Алла", 8);
            gainRomance(player, 8);
            player.setFlag("has_candies", false);
            recordNPCChoice("Алла", "gave_candies_to_alla", 1);
        } else if (selected == "Подарить и цветы, и конфеты") {
            ConsoleUI::RenderScreen("ПОДАРКИ",
                "Ты даришь и цветы, и конфеты. Алла краснеет:\n"
                "«Ты серьёзно так подготовился?.. Спасибо.»",
                {}, player,
                ConsoleUI::GetAllaPortrait(),
                "Алла");
            state.getPlayer().modifyRelation("Алла", 15);
            gainRomance(player, 15);
            player.setFlag("has_flowers", false);
            player.setFlag("has_candies", false);
            recordNPCChoice("Алла", "gave_full_gift_to_alla", 1);
        } else {
            ConsoleUI::RenderScreen("БЕЗ ПОДАРКА",
                "Ты решаешь пока ничего не дарить. Алла ничего не говорит, но момент проходит сухо.",
                {}, player,
                ConsoleUI::GetAllaPortrait(),
                "Алла");
        }
        ConsoleUI::WaitForEnter();
    }

    ConsoleUI::RenderScreen("КУДА ПОЙТИ?",
        "Алла ждёт твоего предложения. Куда пойдёте?",
        {"В кафе — дорого, но уютно",
         "В парк — дешевле и спокойнее",
         "В кино — классика свиданий"},
        player,
        ConsoleUI::GetAllaPortrait(),
        "Алла");

    int choice = ConsoleUI::ReadInt("", 1, 3);

    std::string dateResult;
    int moneyCost = 0;

    if (choice == 1) {
        dateResult =
            "Вы сидите в уютном кафе. Алла рассказывает о мечтах и планах.\n"
            "Разговор выходит живым и тёплым.";
        moneyCost = 400;
    } else if (choice == 2) {
        dateResult =
            "Вы гуляете по парку, кормите уток и смеётесь над тупыми шутками.\n"
            "Вышло просто, но честно.";
        moneyCost = 200;
    } else {
        dateResult =
            "Вы идёте в кино. В зале темно, и Алла берёт тебя за руку.\n"
            "Ты почти не помнишь, о чём был фильм.";
        moneyCost = 300;
    }

    if (player.getStats().money >= scaledCost(player, moneyCost)) {
        spendMoney(player, moneyCost);
        ConsoleUI::RenderScreen("СВИДАНИЕ", dateResult, {}, player, ConsoleUI::GetAllaPortrait(), "Алла");
        state.getPlayer().modifyRelation("Алла", 15);
        gainRomance(player, 12);
        reduceStress(player, 15);
    } else {
        ConsoleUI::RenderScreen("НЕ ХВАТАЕТ ДЕНЕГ",
            "Денег на выбранный вариант не хватает.\n"
            "Вы просто гуляете. Алла говорит, что ей всё равно приятно быть рядом.",
            {}, player,
            ConsoleUI::GetAllaPortrait(),
            "Алла");
        state.getPlayer().modifyRelation("Алла", 8);
        gainRomance(player, 5);
    }
    ConsoleUI::WaitForEnter();

    ConsoleUI::RenderScreen("РАЗГОВОР",
        "Алла спрашивает, что тебе правда интересно в жизни.",
        {"Говорить об учёбе и целях",
         "Расспросить Аллу о её увлечениях",
         "Говорить в основном о себе",
         "Спросить о её планах после университета"},
        player,
        ConsoleUI::GetAllaPortrait(),
        "Алла");

    choice = ConsoleUI::ReadInt("", 1, 4);
    if (choice == 1) {
        state.getPlayer().modifyRelation("Алла", 3);
        gainIntellect(player, 2);
    } else if (choice == 2) {
        state.getPlayer().modifyRelation("Алла", 8);
        gainRomance(player, 4);
        recordNPCChoice("Алла", "asked_about_hobbies", 1);
    } else if (choice == 3) {
        state.getPlayer().modifyRelation("Алла", -5);
        loseRomance(player, 4);
        recordNPCChoice("Алла", "talked_only_about_self", 1);
    } else {
        state.getPlayer().modifyRelation("Алла", 12);
        gainRomance(player, 5);
        recordNPCChoice("Алла", "asked_about_future", 1);
    }
    ConsoleUI::WaitForEnter();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rainDis(1, 100);
    if (rainDis(gen) <= 30) {
        ConsoleUI::RenderScreen("ДОЖДЬ",
            "Неожиданно начинается дождь. Алла поднимает воротник и смеётся:\n"
            "«Ну вот, романтика по расписанию.»",
            {"Проводить Аллу домой",
             "Отдать ей зонт, а самому промокнуть",
             "Пошутить и идти вместе под дождём"},
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла");

        choice = ConsoleUI::ReadInt("", 1, 3);
        if (choice == 1) {
            state.getPlayer().modifyRelation("Алла", 5);
            gainRomance(player, 3);
        } else if (choice == 2) {
            state.getPlayer().modifyRelation("Алла", 12);
            gainRomance(player, 8);
            loseHealth(player, 5);
        } else {
            state.getPlayer().modifyRelation("Алла", 10);
            gainRomance(player, 7);
            reduceStress(player, 10);
        }
        ConsoleUI::WaitForEnter();
    }

    ConsoleUI::RenderScreen("ПРОЩАНИЕ",
        "Свидание подходит к концу. Алла смотрит на тебя с надеждой.\n"
        "Что скажешь?",
        {"Мне было приятно провести с тобой время",
         "Надо будет ещё как-нибудь встретиться",
         "Попрощаться без дополнительных слов"},
        player,
        ConsoleUI::GetAllaPortrait(),
        "Алла");

    choice = ConsoleUI::ReadInt("", 1, 3);
    if (choice == 1) {
        state.getPlayer().modifyRelation("Алла", 5);
        gainRomance(player, 3);
    } else if (choice == 2) {
        state.getPlayer().modifyRelation("Алла", 10);
        gainRomance(player, 8);
    }

    if (player.getStats().romance >= 65 && player.getRelation("Алла") >= 75) {
        ConsoleUI::RenderScreen("ИТОГ СВИДАНИЯ",
            "Алла целует тебя в щёку и быстро убегает в подъезд.\n"
            "Ты стоишь под дождём и улыбаешься как идиот.\n"
            "Романтическая концовка теперь доступна.",
            {}, player,
            ConsoleUI::GetAllaPortrait(),
            "Алла");
        player.setFlag("romantic_ending", true);
    } else {
        ConsoleUI::RenderScreen("ИТОГ СВИДАНИЯ",
            "Вы тепло прощаетесь. Между вами явно стало больше доверия,\n"
            "но до настоящей романтической концовки ещё нужно дотянуть.",
            {}, player,
            ConsoleUI::GetAllaPortrait(),
            "Алла");
    }

    player.setFlag("date_with_alla_scheduled", false);
    player.setFlag("day7_done", true);
    player.getStats().clampAll();
    ConsoleUI::WaitForEnter();
}

void Game::runRetakeExam() {
    Player& player = state.getPlayer();
    Stats& stats = player.getStats();

    std::vector<std::pair<int, std::string>> failedExams;
    const std::vector<std::pair<int, std::string>> exams = {
        {1, "История"},
        {2, "Языки и методы программирования"},
        {3, "Дискретная математика"},
        {4, "Математический анализ"}
    };

    for (const auto& exam : exams) {
        int grade = player.getGrade(exam.first);
        if (grade > 0 && grade < GameConstants::EXAM_PASS_THRESHOLD) {
            failedExams.push_back(exam);
        }
    }

    if (failedExams.empty()) {
        ConsoleUI::RenderScreen("ПЕРЕСДАЧА",
            "По оценкам явных проваленных экзаменов нет.\n"
            "Если долг остался из старого сейва, он будет снят как битый хвост.",
            {}, player);
        if (player.getDebts() > 0) {
            player.removeDebt(player.getDebts());
        }
        ConsoleUI::WaitForEnter();
        return;
    }

    std::vector<std::string> choices;
    for (const auto& exam : failedExams) {
        choices.push_back(exam.second + " — пересдать");
    }
    choices.push_back("Отказаться от пересдачи");

    ConsoleUI::RenderScreen("ПЕРЕСДАЧА",
        "Сегодня последний шанс закрыть один долг.\n"
        "Выбери экзамен, который будешь пересдавать.",
        choices,
        player);

    int choice = ConsoleUI::ReadInt("", 1, static_cast<int>(choices.size()));
    if (choice == static_cast<int>(choices.size())) {
        ConsoleUI::RenderScreen("ОТКАЗ",
            "Ты решил не идти на пересдачу. Долг остаётся висеть.",
            {}, player);
        gainStress(player, 10);
        ConsoleUI::WaitForEnter();
        return;
    }

    int examId = failedExams[choice - 1].first;
    std::string examName = failedExams[choice - 1].second;

    ConsoleUI::RenderScreen("ПОДГОТОВКА К ПЕРЕСДАЧЕ",
        "Как будешь готовиться к пересдаче по предмету: " + examName + "?",
        {"Интенсивно заниматься",
         "Повторить основные темы",
         "Надеяться на удачу"},
        player);

    int prepChoice = ConsoleUI::ReadInt("", 1, 3);
    int chanceBonus = 0;

    if (prepChoice == 1) {
        chanceBonus = 20;
        gainIntellect(player, 8);
        gainFatigue(player, 20);
        loseEnergy(player, 15);
        ConsoleUI::RenderScreen("ПОДГОТОВКА", "Ты жёстко готовился. Голова кипит, но шанс вырос.", {}, player);
    } else if (prepChoice == 2) {
        chanceBonus = 10;
        gainIntellect(player, 4);
        gainFatigue(player, 10);
        ConsoleUI::RenderScreen("ПОДГОТОВКА", "Ты повторил основные темы. Не идеально, но лучше, чем ничего.", {}, player);
    } else {
        chanceBonus = 0;
        ConsoleUI::RenderScreen("ПОДГОТОВКА", "Ты решил положиться на удачу. Сомнительная стратегия.", {}, player);
    }
    ConsoleUI::WaitForEnter();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    int successChance = stats.intellect + chanceBonus + player.getRelation("Преподаватели") / 4 + player.getExamScoreModifier();
    successChance = std::clamp(successChance, 10, 95);
    int roll = dis(gen);

    if (roll <= successChance) {
        int newScore = std::max(GameConstants::EXAM_PASS_THRESHOLD, 60);
        player.setGrade(examId, newScore);
        player.removeDebt();
        reduceStress(player, 20);
        gainHumanity(player, 8);

        ConsoleUI::RenderScreen("ПЕРЕСДАЧА СДАНА",
            "Ты успешно пересдал " + examName + ".\n"
            "Преподаватель сухо кивает: «Ладно, зачтено.»\n"
            "Один долг закрыт.",
            {}, player);
    } else {
        gainStress(player, 20);
        ConsoleUI::RenderScreen("ПЕРЕСДАЧА ПРОВАЛЕНА",
            "Ты не смог пересдать " + examName + ".\n"
            "Преподаватель: «В следующий раз готовьтесь лучше.»",
            {}, player);
    }

    player.getStats().clampAll();
    ConsoleUI::WaitForEnter();
}

void Game::runNormalDay7() {
    Player& player = state.getPlayer();
    Stats& stats = player.getStats();

    ConsoleUI::RenderScreen("СВОБОДНЫЙ ДЕНЬ",
        "Свидания нет, долгов нет. Можно провести день с пользой или просто выдохнуть.",
        {"Встретиться с друзьями",
         "Позвонить маме",
         "Почитать книгу дома",
         "Остаться дома и полностью отдохнуть",
         "Готовиться к компьютерным сетям"},
        player);

    int choice = ConsoleUI::ReadInt("", 1, 5);

    if (choice == 1) {
        ConsoleUI::RenderScreen("ДРУЗЬЯ",
            "Ты встречаешься с Булатом и Семёном. Вечер проходит шумно, но живо.",
            {"Играть в Монополию",
             "Играть в шахматы с Семёном",
             "Играть в Дурака с Булатом"},
            player);
        int gameChoice = ConsoleUI::ReadInt("", 1, 3);
        if (gameChoice == 1) {
            state.getPlayer().modifyRelation("Булат", 5);
            state.getPlayer().modifyRelation("Семён", 5);
            reduceStress(player, 15);
        } else if (gameChoice == 2) {
            state.getPlayer().modifyRelation("Семён", 8);
            gainIntellect(player, 3);
            reduceStress(player, 8);
        } else {
            state.getPlayer().modifyRelation("Булат", 5);
            reduceStress(player, 12);
        }
    } else if (choice == 2) {
        int debts = player.getDebts();
        if (debts > 0) {
            ConsoleUI::RenderScreen("ЗВОНОК МАМЕ",
                "Ты честно говоришь маме, что есть долги.\n"
                "Она переживает, но поддерживает тебя.\n"
                "Становится стыдно, зато немного легче.",
                {}, player);
            gainHumanity(player, 3);
            gainStress(player, 5);
        } else {
            ConsoleUI::RenderScreen("ЗВОНОК МАМЕ",
                "Ты говоришь маме, что сессия почти закрыта.\n"
                "Она радуется и переводит тебе немного денег на отдых.",
                {}, player);
            gainMoney(player, 300);
            gainHumanity(player, 8);
            reduceStress(player, 15);
        }
    } else if (choice == 3) {
        ConsoleUI::RenderScreen("КНИГА",
            "Какую книгу выберешь?",
            {"Искусство программирования",
             "Романтическая классика",
             "Как перестать беспокоиться",
             "Сборник смешных историй"},
            player);
        int bookChoice = ConsoleUI::ReadInt("", 1, 4);
        if (bookChoice == 1) {
            gainIntellect(player, 8);
        } else if (bookChoice == 2) {
            gainRomance(player, 5);
        } else if (bookChoice == 3) {
            reduceStress(player, 15);
        } else {
            reduceStress(player, 10);
        }
    } else if (choice == 4) {
        ConsoleUI::RenderScreen("ДОМАШНИЙ ВЕЧЕР",
            "Ты остаёшься дома: сериал, ванна, нормальный ужин и сон.\n"
            "Не героически, зато полезно.",
            {}, player);
        gainEnergy(player, 25);
        reduceFatigue(player, 20);
        reduceStress(player, 20);
        gainHunger(player, 15);
    } else {
        ConsoleUI::RenderScreen("ПОДГОТОВКА",
            "Ты готовишься к последнему экзамену по компьютерным сетям.",
            {}, player);
        gainIntellect(player, 8);
        gainFatigue(player, 10);
    }

    stats.clampAll();
    ConsoleUI::WaitForEnter();
}

void Game::runDay7() {
    ConsoleUI::PrintDayHeader(7, "Свидание или пересдача");

    Player& player = state.getPlayer();
    bool hasDateScheduled = player.hasFlag("date_with_alla_scheduled") ||
                            player.hasFlag("invited_alla_to_walk") ||
                            (player.getRelation("Алла") >= 60 && player.getStats().romance >= 30);
    bool hasDebts = player.getDebts() > 0;

    if (hasDateScheduled && hasDebts) {
        ConsoleUI::RenderScreen("ВЫБОР ДНЯ",
            "На сегодня есть романтическая ветка с Аллой, но у тебя висят долги.\n"
            "Придётся выбрать, что важнее прямо сейчас.",
            {"Пойти на свидание с Аллой — долги останутся",
             "Пойти на пересдачу — свидание сорвётся"},
            player,
            ConsoleUI::GetAllaPortrait(),
            "Алла");

        int choice = ConsoleUI::ReadInt("", 1, 2);
        if (choice == 1) {
            runDateWithAlla();
        } else {
            ConsoleUI::RenderScreen("ОТМЕНА СВИДАНИЯ",
                "Ты объясняешь Алле, что вынужден идти на пересдачу.\n"
                "Она отвечает холоднее обычного, но не устраивает сцену.",
                {}, player,
                ConsoleUI::GetAllaPortrait(),
                "Алла");
            state.getPlayer().modifyRelation("Алла", -8);
            loseRomance(player, 5);
            player.setFlag("date_with_alla_scheduled", false);
            recordNPCChoice("Алла", "cancelled_date_for_retake", 1);
            ConsoleUI::WaitForEnter();
            runRetakeExam();
        }
    } else if (hasDateScheduled) {
        runDateWithAlla();
    } else if (hasDebts) {
        runRetakeExam();
    } else {
        runNormalDay7();
    }

    eventManager.tryTriggerEvent(player, 7);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 7");
    ConsoleUI::PrintPlayerStats(player);
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 8 — ИТОГИ ====================

void Game::runDay8() {
    ConsoleUI::PrintDayHeader(8, "Последний экзамен и подведение итогов");

    // Последний экзамен
    ConsoleUI::PrintHeader("ПОСЛЕДНИЙ ЭКЗАМЕН — КОМПЬЮТЕРНЫЕ СЕТИ");
    NetworksExam networksExam;
    networksExam.runExam(state.getPlayer());
    ConsoleUI::WaitForEnter();

    // Подведение итогов
    ConsoleUI::PrintHeader("ПОДВЕДЕНИЕ ИТОГОВ");

    const auto& stats = state.getPlayer().getStats();
    std::cout << BOX_TL << std::string(78, BOX_H[0]) << BOX_TR "\n";
    std::cout << BOX_V "  " << std::string(74, ' ') << BOX_V "\n";
    std::cout << BOX_V "  Сессия позади. Давай посмотрим, как всё прошло." << std::string(20, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << std::string(74, ' ') << BOX_V "\n";

    int totalScore = 0;
    const char* examNames[] = {"История", "ЯиМП", "Дискретка", "Матанализ", "Комп.сети"};
    for (int i = 1; i <= 5; i++) {
        int g = state.getPlayer().getGrade(i);
        if (g > 0) {
            std::string line = "  " + std::string(examNames[i-1]) + ": " + std::to_string(g) + " баллов";
            std::cout << BOX_V " " << line << std::string(78 - 3 - static_cast<int>(line.size()), ' ') << BOX_V "\n";
            totalScore += g;
        } else {
            std::string line = "  " + std::string(examNames[i-1]) + ": не сдан";
            std::cout << BOX_V " " << line << std::string(78 - 3 - static_cast<int>(line.size()), ' ') << BOX_V "\n";
        }
    }

    std::cout << BOX_V "  " << std::string(74, ' ') << BOX_V "\n";
    std::string line = "  Итоговые характеристики:";
    std::cout << BOX_V " " << line << std::string(78 - 3 - static_cast<int>(line.size()), ' ') << BOX_V "\n";
    auto printStat = [&](const std::string& name, int val) {
        std::string l = "    " + name + ": " + std::to_string(val);
        std::cout << BOX_V " " << l << std::string(78 - 3 - static_cast<int>(l.size()), ' ') << BOX_V "\n";
    };
    printStat("Интеллект", stats.intellect);
    printStat("Человечность", stats.humanity);
    printStat("Романтика", stats.romance);
    printStat("Отношения с Аллой", state.getPlayer().getRelation("Алла"));
    printStat("Долгов", state.getPlayer().getDebts());
    std::cout << BOX_BL << std::string(78, BOX_H[0]) << BOX_BR "\n";
    ConsoleUI::WaitForEnter();

    // Финальный выбор
    std::cout << BOX_TL << std::string(78, BOX_H[0]) << BOX_TR "\n";
    std::cout << BOX_V " ФИНАЛЬНЫЙ ВЫБОР" << std::string(63, ' ') << BOX_V "\n";
    std::cout << BOX_L << std::string(78, BOX_H[0]) << BOX_R "\n";
    std::cout << BOX_V "  Что ты чувствуешь после сессии?" << std::string(46, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << std::string(74, ' ') << BOX_V "\n";
    std::cout << BOX_V "  1. Гордость — ты справился со всем!" << std::string(36, ' ') << BOX_V "\n";
    std::cout << BOX_V "  2. Облегчение — наконец-то всё кончилось." << std::string(31, ' ') << BOX_V "\n";
    std::cout << BOX_V "  3. Усталость — ты выжат как лимон." << std::string(37, ' ') << BOX_V "\n";
    std::cout << BOX_V "  4. Пустоту — что дальше?" << std::string(44, ' ') << BOX_V "\n";
    std::cout << BOX_V "  " << std::string(74, ' ') << BOX_V "\n";
    std::cout << BOX_V "  Ваш выбор [1-4]: ";
    std::cout << std::string(40, ' ') << BOX_V "\n";
    std::cout << BOX_BL << std::string(78, BOX_H[0]) << BOX_BR "\n";

    // Курсор уже в нужном месте после "Ваш выбор"
    std::cout << "\r                                                                                \r";
    std::cout << "  Ваш выбор: ";
    int finalChoice = ConsoleUI::ReadInt("", 1, 4);

    if (finalChoice == 4 && state.getPlayer().getDebts() >= 3) {
        ConsoleUI::RenderScreen("ПУТЬ",
            "Возможно, тебе стоит взять паузу и подумать...\n"
            "Армия — тоже вариант.",
            {}, state.getPlayer());
        state.getPlayer().setFlag("army_path", true);
        ConsoleUI::WaitForEnter();
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 8);

    GameOverCondition ending = EndingSystem::EvaluateEnding(state.getPlayer());
    state.setGameOverReason(ending);
    state.setPhase(GamePhase::GameOver);
}

// ==================== ЛОКАЦИИ ====================

void Game::handleLocation(LocationID loc) {
    switch (loc) {
    case LocationID::Home: handleHomeLocation(); break;
    case LocationID::University: handleUniversityLocation(); break;
    case LocationID::Street: handleStreetLocation(); break;
    case LocationID::Canteen: handleCanteenLocation(); break;
    case LocationID::Shop: handleShopLocation(); break;
    case LocationID::FlowerShop: handleFlowerShopLocation(); break;
    }
}

void Game::handleHomeLocation() {
    ConsoleUI::PrintHeader("ДОМ");
    ConsoleUI::ShowLocationArt(LocationID::Home);

    ConsoleUI::RenderScreen(
        "ДОМ",
        "Ты дома. Можно отдохнуть, поесть или позаниматься.",
        {
            "Лечь спать",
            "Поесть",
            "Позаниматься",
            "Журнал событий",
            "Энциклопедия NPC",
            "Достижения",
            "Привычки"
        },
        state.getPlayer(),
        "",
        "",
        "Дом"
    );

    int choice = ConsoleUI::ReadInt();

    auto& s = state.getPlayer().getStats();

    switch (choice) {
    case 1:
        gainEnergy(state.getPlayer(), GameConstants::SLEEP_ENERGY_GAIN);
        reduceFatigue(state.getPlayer(), GameConstants::SLEEP_FATIGUE_REDUCE);
        reduceStress(state.getPlayer(), GameConstants::SLEEP_STRESS_REDUCE);

        // Сон тратит сытость.
        loseHunger(state.getPlayer(), GameConstants::SLEEP_HUNGER_LOSS);

        state.getPlayer().advanceTime(480);

        ConsoleUI::RenderScreen(
            "СОН",
            "Ты хорошо выспался.\n"
            "За время сна сытость немного снизилась.",
            {},
            state.getPlayer(),
            "",
            "",
            "Дом"
        );
        break;

    case 2:
        if (s.hunger >= GameConstants::MAX_HUNGER) {
            ConsoleUI::RenderScreen(
                "ЕДА",
                "Ты и так полностью сыт.",
                {},
                state.getPlayer(),
                "",
                "",
                "Дом"
            );
        } else if (s.money < scaledCost(state.getPlayer(), GameConstants::EAT_MONEY_COST)) {
            ConsoleUI::RenderScreen(
                "ЕДА",
                "Не хватает денег на еду.",
                {},
                state.getPlayer(),
                "",
                "",
                "Дом"
            );
        } else {
            HungerSystem::Eat(state.getPlayer(), GameConstants::EAT_MONEY_COST);

            ConsoleUI::RenderScreen(
                "ЕДА",
                "Ты поел.\n"
                "Сытость: " + std::to_string(state.getPlayer().getStats().hunger) + "/100.",
                {},
                state.getPlayer(),
                "",
                "",
                "Дом"
            );
        }
        break;

    case 3:
        gainIntellect(state.getPlayer(), GameConstants::STUDY_INTELLECT_GAIN);
        gainFatigue(state.getPlayer(), GameConstants::STUDY_FATIGUE_COST);
        loseEnergy(state.getPlayer(), GameConstants::STUDY_ENERGY_COST);
        state.getPlayer().getStats().motivation += state.getPlayer().scaleGain(3);
        state.getPlayer().getStats().confidence += state.getPlayer().scaleGain(2);
        habits.recordDay("study_daily", state.getPlayer().getCurrentDay());
        addJournalEntry("Дом", "Тимур позанимался дома.", "study", 3);

        state.getPlayer().advanceTime(120);

        ConsoleUI::RenderScreen(
            "ЗАНЯТИЯ",
            "Ты позанимался.\n"
            "Интеллект вырос, но ты устал.",
            {},
            state.getPlayer(),
            "",
            "",
            "Дом"
        );
        break;

    case 4:
        showJournal();
        break;

    case 5:
        showEncyclopedia();
        break;

    case 6:
        showAchievements();
        break;

    case 7:
        showHabits();
        break;

    default:
        ConsoleUI::RenderScreen(
            "ДОМ",
            "Ты ничего не сделал.",
            {},
            state.getPlayer(),
            "",
            "",
            "Дом"
        );
        break;
    }

    s.clampAll();
    checkAchievements();

    if (choice < 4 || choice > 7) {
        ConsoleUI::WaitForEnter();
    }
}

void Game::handleUniversityLocation() {
    ConsoleUI::PrintHeader("УНИВЕРСИТЕТ");
    ConsoleUI::ShowLocationArt(LocationID::University);
    ConsoleUI::RenderScreen("УНИВЕРСИТЕТ", "Кого хочешь встретить?",
        {"Аллу", "Булата", "Семёна", "Артёма", "Пойти в библиотеку"},
        state.getPlayer());

    int choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1: interactWithAlla(); break;
    case 2: interactWithBulat(); break;
    case 3: interactWithSemen(); break;
    case 4: interactWithArtem(); break;
    case 5:
        ConsoleUI::RenderScreen("БИБЛИОТЕКА", "Ты занимаешься в библиотеке.", {}, state.getPlayer());
        gainIntellect(state.getPlayer(), 5);
        state.getPlayer().getStats().motivation += state.getPlayer().scaleGain(2);
        habits.recordDay("study_daily", state.getPlayer().getCurrentDay());
        encyclopedia.discover("Преподаватели");
        addJournalEntry("Университет", "Тимур занимался в библиотеке.", "study", 3);
        state.getPlayer().advanceTime(120);
        ConsoleUI::WaitForEnter();
        break;
    }
    state.getPlayer().advanceTime(30);
}

void Game::handleStreetLocation() {
    ConsoleUI::PrintHeader("УЛИЦА");
    ConsoleUI::ShowLocationArt(LocationID::Street);
    ConsoleUI::RenderScreen("УЛИЦА", "Куда пойти?",
        {"В университет", "В столовую", "В магазин", "В цветочный магазин", "Просто гулять"},
        state.getPlayer());

    int choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1: state.getPlayer().setLocation(LocationID::University); break;
    case 2: state.getPlayer().setLocation(LocationID::Canteen); break;
    case 3: state.getPlayer().setLocation(LocationID::Shop); break;
    case 4: state.getPlayer().setLocation(LocationID::FlowerShop); break;
    case 5:
        ConsoleUI::RenderScreen("ПРОГУЛКА", "Ты гуляешь. Встречаешь знакомых.", {}, state.getPlayer());
        reduceStress(state.getPlayer(), 5);
        state.getPlayer().getStats().anxiety -= state.getPlayer().scaleGain(2);
        habits.recordDay("walk_evening", state.getPlayer().getCurrentDay());
        addJournalEntry("Улица", "Тимур вышел на прогулку.", "event", 2);
        state.getPlayer().advanceTime(30);
        ConsoleUI::WaitForEnter();
        break;
    }
}

void Game::handleCanteenLocation() {
    ConsoleUI::PrintHeader("СТОЛОВАЯ");
    ConsoleUI::ShowLocationArt(LocationID::Canteen);

    int lunchCost = scaledCost(state.getPlayer(), 150);
    int snackCost = scaledCost(state.getPlayer(), 50);
    int lunchRestore = state.getPlayer().scaleGain(50);
    int snackRestore = state.getPlayer().scaleGain(20);

    ConsoleUI::RenderScreen(
        "СТОЛОВАЯ",
        "Что будешь заказывать?",
        {
            "Комплексный обед (" + std::to_string(lunchCost) + " руб, +" + std::to_string(lunchRestore) + " сытости)",
            "Чай с булочкой (" + std::to_string(snackCost) + " руб, +" + std::to_string(snackRestore) + " сытости)",
            "Ничего, просто посидеть"
        },
        state.getPlayer(),
        "",
        "",
        "Столовая"
    );

    int choice = ConsoleUI::ReadInt();

    auto& s = state.getPlayer().getStats();

    switch (choice) {
    case 1:
        if (s.money >= lunchCost) {
            spendMoney(state.getPlayer(), 150);
            gainHunger(state.getPlayer(), 50);
            gainEnergy(state.getPlayer(), 15);
            habits.recordDay("no_junk", state.getPlayer().getCurrentDay());
            addJournalEntry("Столовая", "Тимур нормально пообедал.", "health", 2);

            ConsoleUI::RenderScreen(
                "ОБЕД",
                "Вкусный обед!\n"
                "Сытость восстановлена.",
                {},
                state.getPlayer(),
                "",
                "",
                "Столовая"
            );
        } else {
            ConsoleUI::RenderScreen(
                "ОБЕД",
                "Не хватает денег.",
                {},
                state.getPlayer(),
                "",
                "",
                "Столовая"
            );
        }
        break;

    case 2:
        if (s.money >= snackCost) {
            spendMoney(state.getPlayer(), 50);
            gainHunger(state.getPlayer(), 20);
            gainEnergy(state.getPlayer(), 5);
            habits.recordDay("junk_food", state.getPlayer().getCurrentDay());

            ConsoleUI::RenderScreen(
                "ПЕРЕКУС",
                "Чай с булочкой — отлично.\n"
                "Сытость немного восстановлена.",
                {},
                state.getPlayer(),
                "",
                "",
                "Столовая"
            );
        } else {
            ConsoleUI::RenderScreen(
                "ПЕРЕКУС",
                "Не хватает денег.",
                {},
                state.getPlayer(),
                "",
                "",
                "Столовая"
            );
        }
        break;

    case 3:
        ConsoleUI::RenderScreen(
            "ОТДЫХ",
            "Ты просто отдыхаешь.",
            {},
            state.getPlayer(),
            "",
            "",
            "Столовая"
        );
        reduceStress(state.getPlayer(), 3);
        break;

    default:
        ConsoleUI::RenderScreen(
            "СТОЛОВАЯ",
            "Ты ничего не заказал.",
            {},
            state.getPlayer(),
            "",
            "",
            "Столовая"
        );
        break;
    }

    state.getPlayer().advanceTime(30);
    s.clampAll();
    ConsoleUI::WaitForEnter();
}

void Game::handleShopLocation() {
    ConsoleUI::PrintHeader("МАГАЗИН");
    ConsoleUI::ShowLocationArt(LocationID::Shop);

    int foodCost = scaledCost(state.getPlayer(), 200);
    int stationeryCost = scaledCost(state.getPlayer(), 100);
    int bookCost = scaledCost(state.getPlayer(), 300);
    int foodRestore = state.getPlayer().scaleGain(30);
    int stationeryGain = state.getPlayer().scaleGain(2);
    int bookGain = state.getPlayer().scaleGain(8);

    ConsoleUI::RenderScreen(
        "МАГАЗИН",
        "Что купить?",
        {
            "Продукты (" + std::to_string(foodCost) + " руб, +" + std::to_string(foodRestore) + " сытости)",
            "Канцтовары (" + std::to_string(stationeryCost) + " руб, +" + std::to_string(stationeryGain) + " интеллект)",
            "Книгу по C++ (" + std::to_string(bookCost) + " руб, +" + std::to_string(bookGain) + " интеллект)",
            "Выйти"
        },
        state.getPlayer(),
        "",
        "",
        "Магазин"
    );

    int choice = ConsoleUI::ReadInt();

    auto& s = state.getPlayer().getStats();

    switch (choice) {
    case 1:
        if (s.money >= foodCost) {
            spendMoney(state.getPlayer(), 200);
            gainHunger(state.getPlayer(), 30);

            ConsoleUI::RenderScreen(
                "ПОКУПКА",
                "Купил продукты.\n"
                "Сытость немного восстановлена.",
                {},
                state.getPlayer(),
                "",
                "",
                "Магазин"
            );
        } else {
            ConsoleUI::RenderScreen(
                "ПОКУПКА",
                "Не хватает денег.",
                {},
                state.getPlayer(),
                "",
                "",
                "Магазин"
            );
        }
        break;

    case 2:
        if (s.money >= stationeryCost) {
            spendMoney(state.getPlayer(), 100);
            gainIntellect(state.getPlayer(), 2);

            ConsoleUI::RenderScreen(
                "ПОКУПКА",
                "Купил канцтовары.",
                {},
                state.getPlayer(),
                "",
                "",
                "Магазин"
            );
        } else {
            ConsoleUI::RenderScreen(
                "ПОКУПКА",
                "Не хватает денег.",
                {},
                state.getPlayer(),
                "",
                "",
                "Магазин"
            );
        }
        break;

    case 3:
        if (s.money >= bookCost) {
            spendMoney(state.getPlayer(), 300);
            gainIntellect(state.getPlayer(), 8);
            state.getPlayer().getStats().confidence += state.getPlayer().scaleGain(2);
            addJournalEntry("Магазин", "Тимур купил книгу по C++.", "study", 3);

            ConsoleUI::RenderScreen(
                "ПОКУПКА",
                "Купил книгу по C++.\n"
                "Теперь ты почти гуру.",
                {},
                state.getPlayer(),
                "",
                "",
                "Магазин"
            );
        } else {
            ConsoleUI::RenderScreen(
                "ПОКУПКА",
                "Не хватает денег.",
                {},
                state.getPlayer(),
                "",
                "",
                "Магазин"
            );
        }
        break;

    case 4:
        ConsoleUI::RenderScreen(
            "МАГАЗИН",
            "Ты вышел из магазина.",
            {},
            state.getPlayer(),
            "",
            "",
            "Магазин"
        );
        break;

    default:
        ConsoleUI::RenderScreen(
            "МАГАЗИН",
            "Ты ничего не купил.",
            {},
            state.getPlayer(),
            "",
            "",
            "Магазин"
        );
        break;
    }

    state.getPlayer().advanceTime(20);
    s.clampAll();
    ConsoleUI::WaitForEnter();
}

void Game::handleFlowerShopLocation() {
    ConsoleUI::PrintHeader("ЦВЕТОЧНЫЙ МАГАЗИН");
    ConsoleUI::ShowLocationArt(LocationID::FlowerShop);

    int flowerCost = scaledCost(state.getPlayer(), GameConstants::FLOWER_COST);

    if (state.getPlayer().getStats().money >= flowerCost) {
        ConsoleUI::RenderScreen("ЦВЕТЫ",
            "В цветочном магазине прекрасный аромат.\n"
            "Продавщица: «Что желаете?»\n"
            "Букет роз стоит " + std::to_string(flowerCost) + " руб.",
            {"Купить букет цветов (" + std::to_string(flowerCost) + " руб)", "Выйти"},
            state.getPlayer());

        int choice = ConsoleUI::ReadInt();

        if (choice == 1) {
            spendMoney(state.getPlayer(), GameConstants::FLOWER_COST);
            state.getPlayer().setFlag("has_flowers", true);
            ConsoleUI::RenderScreen("ЦВЕТЫ",
                "Ты купил прекрасный букет!\n"
                "Кому подарить?",
                {"Подарить Алле", "Оставить себе"},
                state.getPlayer());

            int subChoice = ConsoleUI::ReadInt();

            if (subChoice == 1) {
                state.getPlayer().modifyRelation("Алла", 15);
                gainRomance(state.getPlayer(), 10);
                state.getPlayer().getStats().confidence += state.getPlayer().scaleGain(3);
                state.getPlayer().setFlag("gave_flowers_to_alla", true);
                encyclopedia.discover("Алла");
                addJournalEntry("Цветочный магазин", "Тимур подарил Алле цветы.", "npc", 4);
                ConsoleUI::RenderScreen("ЦВЕТЫ", "Алла в восторге!", {}, state.getPlayer());
            } else {
                ConsoleUI::RenderScreen("ЦВЕТЫ", "Цветы будут стоять у тебя дома.", {}, state.getPlayer());
            }
            ConsoleUI::WaitForEnter();
        }
    } else {
        ConsoleUI::RenderScreen("ЦВЕТЫ", "У тебя не хватает денег на цветы.", {}, state.getPlayer());
        ConsoleUI::WaitForEnter();
    }
    state.getPlayer().advanceTime(15);
}

// ==================== ВЗАИМОДЕЙСТВИЕ С NPC ====================

void Game::interactWithAlla() {
    encyclopedia.discover("Алла");
    habits.recordDay("socialize", state.getPlayer().getCurrentDay());
    addJournalEntry("Университет", "Тимур поговорил с Алла.", "npc", 3);
    ConsoleUI::PrintHeader("ОБЩЕНИЕ С АЛЛОЙ");
    ConsoleUI::ShowNPCPortrait("Алла");
    std::cout << BOX_V " " << alla->getDialog(state.getPlayer()) << "\n";
    ConsoleUI::PrintSeparator();

    auto choices = alla->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << BOX_V " " << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << BOX_V " 0. Закончить разговор\n";
    std::cout << BOX_BL << std::string(78, BOX_H[0]) << BOX_BR "\n";
    std::cout << BOX_V " Ваш выбор: ";

    int choice = ConsoleUI::ReadInt("", 0, static_cast<int>(choices.size()));

    if (choice > 0 && choice <= static_cast<int>(choices.size())) {
        const auto& selected = choices[choice - 1];
        RelationshipSystem::ApplyChoiceEffect(state.getPlayer(), "Алла", selected.effects);
        std::cout << "\n" << selected.resultingText << "\n";

        if (selected.text.find("Пригласить на свидание") != std::string::npos) {
            ConsoleUI::WaitForEnter();
            ConsoleUI::PrintHeader("СВИДАНИЕ");
            std::cout << BOX_V " Алла соглашается! Вы договариваетесь о встрече.\n";
            state.getPlayer().setFlag("day7_available", true);
        }
    }

    checkAchievements();
    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}

void Game::interactWithBulat() {
    encyclopedia.discover("Булат");
    habits.recordDay("socialize", state.getPlayer().getCurrentDay());
    addJournalEntry("Университет", "Тимур поговорил с Булат.", "npc", 3);
    ConsoleUI::PrintHeader("ОБЩЕНИЕ С БУЛАТОМ");
    ConsoleUI::ShowNPCPortrait("Булат");
    std::cout << BOX_V " " << bulat->getDialog(state.getPlayer()) << "\n";
    ConsoleUI::PrintSeparator();

    auto choices = bulat->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << BOX_V " " << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << BOX_V " 0. Закончить разговор\n";
    std::cout << BOX_BL << std::string(78, BOX_H[0]) << BOX_BR "\n";
    std::cout << BOX_V " Ваш выбор: ";

    int choice = ConsoleUI::ReadInt("", 0, static_cast<int>(choices.size()));

    if (choice > 0 && choice <= static_cast<int>(choices.size())) {
        const auto& selected = choices[choice - 1];
        RelationshipSystem::ApplyChoiceEffect(state.getPlayer(), "Булат", selected.effects);
        std::cout << "\n" << selected.resultingText << "\n";
    }

    checkAchievements();
    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}

void Game::interactWithSemen() {
    encyclopedia.discover("Семён");
    habits.recordDay("socialize", state.getPlayer().getCurrentDay());
    addJournalEntry("Университет", "Тимур поговорил с Семён.", "npc", 3);
    ConsoleUI::PrintHeader("ОБЩЕНИЕ С СЕМЁНОМ");
    ConsoleUI::ShowNPCPortrait("Семён");
    std::cout << BOX_V " " << semen->getDialog(state.getPlayer()) << "\n";
    ConsoleUI::PrintSeparator();

    auto choices = semen->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << BOX_V " " << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << BOX_V " 0. Закончить разговор\n";
    std::cout << BOX_BL << std::string(78, BOX_H[0]) << BOX_BR "\n";
    std::cout << BOX_V " Ваш выбор: ";

    int choice = ConsoleUI::ReadInt("", 0, static_cast<int>(choices.size()));

    if (choice > 0 && choice <= static_cast<int>(choices.size())) {
        const auto& selected = choices[choice - 1];
        RelationshipSystem::ApplyChoiceEffect(state.getPlayer(), "Семён", selected.effects);
        std::cout << "\n" << selected.resultingText << "\n";
    }

    checkAchievements();
    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}

void Game::interactWithArtem() {
    encyclopedia.discover("Артём");
    habits.recordDay("socialize", state.getPlayer().getCurrentDay());
    addJournalEntry("Университет", "Тимур поговорил с Артём.", "npc", 3);
    ConsoleUI::PrintHeader("ОБЩЕНИЕ С АРТЁМОМ");
    ConsoleUI::ShowNPCPortrait("Артём");
    std::cout << BOX_V " " << artem->getDialog(state.getPlayer()) << "\n";
    ConsoleUI::PrintSeparator();

    auto choices = artem->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << BOX_V " " << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << BOX_V " 0. Закончить разговор\n";
    std::cout << BOX_BL << std::string(78, BOX_H[0]) << BOX_BR "\n";
    std::cout << BOX_V " Ваш выбор: ";

    int choice = ConsoleUI::ReadInt("", 0, static_cast<int>(choices.size()));

    if (choice > 0 && choice <= static_cast<int>(choices.size())) {
        const auto& selected = choices[choice - 1];
        RelationshipSystem::ApplyChoiceEffect(state.getPlayer(), "Артём", selected.effects);
        std::cout << "\n" << selected.resultingText << "\n";
    }

    checkAchievements();
    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}
