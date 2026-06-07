#include "Game.h"
#include "../ui/ConsoleUI.h"
#include "../ui/Menu.h"
#include "../ui/DevMode.h"
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
}

void Game::checkGameOver() {
    auto reason = EndingSystem::CheckEnding(state.getPlayer());
    if (reason != GameOverCondition::None) {
        state.setGameOverReason(reason);
        state.setPhase(GamePhase::GameOver);
    }
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
    ConsoleUI::PrintDayHeader(1, "История — первый экзамен");

    // Утро
    {
        std::string morningText =
            "Понедельник. Солнце только начинает пробиваться сквозь\n"
            "шторы твоей комнаты в общежитии. За окном слышен шум\n"
            "утреннего города — где-то сигналит машина, лает собака.\n"
            "Разбудил тебя не будильник, а громкий стук в дверь.\n\n"
            "Булат: «" + state.getPlayer().getName() + ", вставай! Через час экзамен!\n"
            "Я уже готов, а ты ещё дрыхнешь! Соня несчастная!»\n\n"
            "Ты смотришь на часы — 7:30. Первый экзамен в 9:00.\n"
            "Сердце начинает биться быстрее. Сегодня всё решается.";
        ConsoleUI::RenderScreen("УТРО", morningText,
            {"Вскочить и быстро собраться за 5 минут",
             "Попросить Булата подождать, собираться не спеша",
             "Сказать, что идёшь один, пусть не ждёт"},
            state.getPlayer(), ConsoleUI::GetBulatPortrait(), "Булат");
    }

    int choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1:
        ConsoleUI::RenderScreen("УТРО", "Ты вскакиваешь, быстро умываешься, хватаешь конспект.\nБулат одобрительно свистит: «Молоток! Скорость — наше всё!»\nВы выбегаете вместе.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 5);
        loseEnergy(state.getPlayer(), 5);
        break;
    case 2:
        ConsoleUI::RenderScreen("УТРО", "Ты просишь Булата подождать. Пока ты собираешься,\nвы успеваете обсудить историю. Булат рассказывает\nпару интересных фактов про Древнюю Русь.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 7);
        gainIntellect(state.getPlayer(), 3);
        break;
    case 3:
        ConsoleUI::RenderScreen("УТРО", "Булат уходит один. Ты чувствуешь лёгкую неловкость.\nНо хотя бы есть время спокойно собраться.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", -3);
        break;
    default:
        ConsoleUI::RenderScreen("УТРО", "Булат уходит, пожимая плечами.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", -5);
        break;
    }
    ConsoleUI::WaitForEnter();

    // В коридоре перед экзаменом
    {
        std::string hallText =
            "Коридор университета встречает тебя гулом голосов.\n"
            "Возле 215-й аудитории толпятся студенты. Кто-то\n"
            "лихорадочно листает конспекты, кто-то шепчет молитвы.\n"
            "В воздухе пахнет кофе и канцелярией. На стенах —\n"
            "стенды с расписанием и объявление о карантине.\n\n"
            "Алла подходит к тебе с взволнованным лицом. Она\n"
            "явно не спала полночи — под глазами круги, но\n"
            "она всё равно улыбается:\n"
            "«Привет! Я так переживаю... Ты готов?\n"
            "Говорят, Елена Викторовна сегодня в плохом настроении.\n"
            "Вчера она полгруппы отправила на пересдачу!»";
        ConsoleUI::RenderScreen("ПЕРЕД ЭКЗАМЕНОМ", hallText,
            {"Успокоить Аллу: «Всё будет хорошо, ты подготовилась»",
             "Признаться, что сам не готов и паникуешь",
             "Попросить Аллу помочь с последними вопросами",
             "Игнорировать — сейчас не до разговоров"},
            state.getPlayer(), ConsoleUI::GetAllaPortrait(), "Алла");
    }

    choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1:
        ConsoleUI::RenderScreen("АЛЛА", "Алла улыбается: «Спасибо, мне стало спокойнее.\nТы хороший друг.»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", 8);
        gainRomance(state.getPlayer(), 3);
        recordNPCChoice("Алла", "helped_player", 1);
        break;
    case 2:
        ConsoleUI::RenderScreen("АЛЛА", "Алла вздыхает: «Мы оба в одной лодке...\nЛадно, удачи нам!»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", 2);
        break;
    case 3:
        ConsoleUI::RenderScreen("АЛЛА", "Алла быстро пересказывает ключевые даты.\nТы чувствуешь, что стало чуть понятнее.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", 10);
        gainIntellect(state.getPlayer(), 4);
        recordNPCChoice("Алла", "helped_player", 1);
        break;
    case 4:
        ConsoleUI::RenderScreen("АЛЛА", "Алла отворачивается. Кажется, она обиделась.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", -5);
        recordNPCChoice("Алла", "was_rude", 1);
        break;
    }
    ConsoleUI::WaitForEnter();

    // Экзамен по истории
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО ИСТОРИИ");
    HistoryExam historyExam;
    historyExam.runExam(state.getPlayer());
    ConsoleUI::WaitForEnter();

    // После экзамена
    {
        std::string afterText =
            "Экзамен позади. Ты выходишь из аудитории, чувствуя\n"
            "невероятное облегчение. Солнце в коридоре кажется\n"
            "ярче, чем было утром. Кто-то из группы обсуждает\n"
            "вопросы билетов, кто-то уже спорит с преподавателем.\n\n"
            "В коридоре тебя встречает Алла — она сдала раньше.\n"
            "Она нервно теребит край тетради:\n"
            "«Ну как? Рассказывай! Я чуть не умерла от ожидания!»";
        ConsoleUI::RenderScreen("ПОСЛЕ ЭКЗАМЕНА", afterText,
            {"Сказать, что всё отлично — лёгкий был экзамен",
             "Признаться, что было трудно, но справился",
             "Отмахнуться: «Да норм, бывало и хуже»"},
            state.getPlayer());
    }

    choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1:
        state.getPlayer().modifyRelation("Алла", 3);
        gainHumanity(state.getPlayer(), 2);
        break;
    case 2:
        state.getPlayer().modifyRelation("Алла", 5);
        break;
    case 3:
        state.getPlayer().modifyRelation("Алла", -2);
        loseHumanity(state.getPlayer(), 2);
        break;
    }
    ConsoleUI::WaitForEnter();

    eventManager.tryTriggerEvent(state.getPlayer(), 1);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 1");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 2 — ЯИМП ====================

void Game::runDay2() {
    ConsoleUI::PrintDayHeader(2, "Язык и математика программирования");

    {
        std::string text =
            "Второй экзамен — Языки и методы программирования.\n"
            "Михаил Олегович известен своими каверзными вопросами про C++.\n"
            "Ты подходишь к университету и замечаешь Артёма,\n"
            "который сидит на скамейке с ноутбуком.";
        ConsoleUI::RenderScreen("ЭКЗАМЕН ЯИМП", text,
            {"Подойти к Артёму, поздороваться",
             "Пройти мимо — сейчас не до разговоров",
             "Спросить у Артёма про экзамен"},
            state.getPlayer(), ConsoleUI::GetArtemPortrait(), "Артём");
    }

    int choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1: {
        ConsoleUI::RenderScreen("АРТЁМ",
            "Артём поднимает взгляд: «А, привет! Готов к плюсам?»\n"
            "Он показывает тебе ноутбук с открытым кодом.\n"
            "«Смотри, я написал шаблонный умный указатель.\n"
            "Красота же, правда?»",
            {"Посмотреть код и похвалить",
             "Сказать, что сейчас не до программирования",
             "Попросить объяснить шаблоны"},
            state.getPlayer(), ConsoleUI::GetArtemPortrait(), "Артём");

        choice = ConsoleUI::ReadInt();

        if (choice == 1) {
            state.getPlayer().modifyRelation("Артём", 5);
            gainIntellect(state.getPlayer(), 2);
            recordNPCChoice("Артём", "helped_player", 1);
        } else if (choice == 2) {
            state.getPlayer().modifyRelation("Артём", -3);
        } else if (choice == 3) {
            state.getPlayer().modifyRelation("Артём", 8);
            gainIntellect(state.getPlayer(), 5);
            recordNPCChoice("Артём", "helped_player", 1);
        }
        ConsoleUI::WaitForEnter();
        break;
    }
    case 2:
        state.getPlayer().modifyRelation("Артём", -2);
        break;
    case 3: {
        ConsoleUI::RenderScreen("АРТЁМ",
            "Артём: «Экзамен? Да брось, это всё фигня.\n"
            "Главное — понимать концепции, а не зубрить.\n"
            "Вот смотри: полиморфизм — это...»\n"
            "Он увлекается и объясняет 15 минут.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Артём", 5);
        gainIntellect(state.getPlayer(), 3);
        recordNPCChoice("Артём", "helped_player", 1);
        ConsoleUI::WaitForEnter();
        break;
    }
    }
    ConsoleUI::WaitForEnter();

    // Экзамен ЯиМП
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО ЯИМП");
    YAMPExam yampExam;
    yampExam.runExam(state.getPlayer());

    // После экзамена
    {
        std::string afterText =
            "После экзамена ты чувствуешь усталость.\n"
            "В вестибюле сталкиваешься с Семёном.\n"
            "Семён: «Ну как, сдал? А то у меня есть\n"
            "интересная информация на будущее...»";
        ConsoleUI::RenderScreen("ПОСЛЕ ЭКЗАМЕНА", afterText,
            {"Расспросить Семёна про информацию",
             "Сказать, что устал, и пойти домой",
             "Предложить Семёну пойти поесть"},
            state.getPlayer(), ConsoleUI::GetSemenPortrait(), "Семён");
    }

    choice = ConsoleUI::ReadInt();

    if (choice == 1) {
        ConsoleUI::RenderScreen("СЕМЁН",
            "Семён понижает голос:\n"
            "«Короче, у меня есть контакты, где можно купить\n"
            "ответы на матанализ. Если что — обращайся.»",
            {}, state.getPlayer());
        state.getPlayer().setFlag("knows_about_answers", true);
        recordNPCChoice("Семён", "shared_info", 1);
        ConsoleUI::WaitForEnter();
    } else if (choice == 3) {
        ConsoleUI::RenderScreen("СТОЛОВАЯ",
            "Вы идёте в столовую. Семён рассказывает\n"
            "байки про прошлые сессии.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Семён", 5);
        HungerSystem::Eat(state.getPlayer(), 100);
        ConsoleUI::WaitForEnter();
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 2);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 2");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 3 — ПОДГОТОВКА ====================

void Game::runDay3() {
    ConsoleUI::PrintDayHeader(3, "Подготовка к дискретной математике");

    {
        std::string text =
            "Третий день. Экзамена нет — чистая подготовка.\n"
            "Завтра дискретная математика, один из самых сложных предметов.\n"
            "Нужно решить, чем заняться сегодня.";
        ConsoleUI::RenderScreen("ДЕНЬ ПОДГОТОВКИ", text,
            {"Пойти в библиотеку готовиться к дискретке",
             "Пойти на физкультуру (снять стресс)",
             "Пойти на 3D-моделирование (доп. занятие)",
             "Встретиться с Семёном и Аллой"},
            state.getPlayer());
    }

    int choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1: {
        ConsoleUI::RenderScreen("БИБЛИОТЕКА",
            "Ты сидишь в библиотеке, штудируешь графы и матрицы.\n"
            "Голова идёт кругом, но ты чувствуешь прогресс.",
            {}, state.getPlayer());
        gainIntellect(state.getPlayer(), 8);
        gainFatigue(state.getPlayer(), 15);
        state.getPlayer().advanceTime(180);
        break;
    }
    case 2: {
        ConsoleUI::RenderScreen("ФИЗКУЛЬТУРА",
            "Ты идёшь в спортзал. Полчаса бега, турник, растяжка.\n"
            "Пот пробил, но в голове прояснилось.",
            {}, state.getPlayer());
        reduceStress(state.getPlayer(), 15);
        gainEnergy(state.getPlayer(), 10);
        gainFatigue(state.getPlayer(), 10);
        state.getPlayer().advanceTime(90);
        break;
    }
    case 3: {
        ConsoleUI::RenderScreen("3D-МОДЕЛИРОВАНИЕ",
            "Ты идёшь на факультатив по 3D-моделированию.\n"
            "Преподаватель показывает Blender.\n"
            "Ты создаёшь примитивную модель чайника.\n"
            "Это отвлекает от мыслей об экзаменах.",
            {}, state.getPlayer());
        reduceStress(state.getPlayer(), 10);
        gainIntellect(state.getPlayer(), 3);
        state.getPlayer().advanceTime(120);
        break;
    }
    case 4: {
        ConsoleUI::RenderScreen("ВСТРЕЧА",
            "Ты встречаешься с Семёном и Аллой в кафе.\n"
            "Семён раздаёт советы, Алла смеётся над его шутками.\n"
            "Вы обсуждаете предстоящие экзамены.",
            {"Обсуждать учёбу и стратегию сдачи",
             "Подшучивать над Семёном, флиртовать с Аллой",
             "Просить помощи у обоих"},
            state.getPlayer());

        choice = ConsoleUI::ReadInt();

        if (choice == 1) {
            gainIntellect(state.getPlayer(), 3);
            state.getPlayer().modifyRelation("Семён", 3);
            state.getPlayer().modifyRelation("Алла", 3);
        } else if (choice == 2) {
            state.getPlayer().modifyRelation("Алла", 5);
            state.getPlayer().modifyRelation("Семён", 2);
            gainRomance(state.getPlayer(), 3);
            reduceStress(state.getPlayer(), 5);
        } else {
            gainIntellect(state.getPlayer(), 5);
            state.getPlayer().modifyRelation("Семён", 5);
            state.getPlayer().modifyRelation("Алла", 5);
            recordNPCChoice("Алла", "helped_player", 1);
            recordNPCChoice("Семён", "helped_player", 1);
        }
        state.getPlayer().advanceTime(120);
        break;
    }
    }
    ConsoleUI::WaitForEnter();

    eventManager.tryTriggerEvent(state.getPlayer(), 3);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 3");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 4 — ДИСКРЕТНАЯ МАТЕМАТИКА ====================

void Game::runDay4() {
    ConsoleUI::PrintDayHeader(4, "Дискретная математика");

    // Ночь перед экзаменом
    {
        std::string nightText =
            "Ночь перед экзаменом по дискретной математике.\n"
            "Ты ворочаешься в постели. Сон не идёт.\n"
            "Мысли путаются: графы, деревья, булевы функции...";
        ConsoleUI::RenderScreen("НОЧЬ ПЕРЕД ЭКЗАМЕНОМ", nightText,
            {"Встать и ещё раз пролистать конспект",
             "Остаться в постели — здоровье важнее",
             "Включить игры на телефоне, чтобы отвлечься"},
            state.getPlayer());
    }

    int choice = ConsoleUI::ReadInt();

    if (choice == 1) {
        ConsoleUI::RenderScreen("НОЧЬ", "Ты включаешь свет и читаешь конспект до 2 часов ночи.",
            {}, state.getPlayer());
        gainIntellect(state.getPlayer(), 5);
        gainFatigue(state.getPlayer(), 15);
        loseEnergy(state.getPlayer(), 10);
    } else if (choice == 2) {
        ConsoleUI::RenderScreen("НОЧЬ", "Ты закрываешь глаза и проваливаешься в сон.",
            {}, state.getPlayer());
        gainEnergy(state.getPlayer(), 15);
    } else {
        ConsoleUI::RenderScreen("НОЧЬ", "Ты играешь в мобильную игру до часу ночи.\nУтром жалеешь об этом.",
            {}, state.getPlayer());
        loseEnergy(state.getPlayer(), 5);
        reduceStress(state.getPlayer(), 3);
        gainFatigue(state.getPlayer(), 5);
    }
    ConsoleUI::WaitForEnter();

    // Утро. Булат
    {
        std::string morningText =
            "Утром тебя встречает Булат.\n"
            "Он выглядит встревоженным:\n"
            "«Слышал, у Натальи Петровны сегодня разнос.\n"
            "Говорят, половина группы завалит.»";
        ConsoleUI::RenderScreen("УТРО С БУЛАТОМ", morningText,
            {"Спросить, нужна ли Булату помощь",
             "Сказать, что сам в панике",
             "Пошутить, чтобы разрядить обстановку"},
            state.getPlayer(), ConsoleUI::GetBulatPortrait(), "Булат");
    }

    choice = ConsoleUI::ReadInt();

    if (choice == 1) {
        ConsoleUI::RenderScreen("БУЛАТ", "Булат благодарит: «Спасибо, брат! Ты настоящий друг.\nДавай вместе повторим определения.»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 10);
        gainIntellect(state.getPlayer(), 2);
        recordNPCChoice("Булат", "helped_player", 1);
    } else if (choice == 2) {
        ConsoleUI::RenderScreen("БУЛАТ", "Булат вздыхает: «Ну, мы вместе прорвёмся... или вместе завалим.»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 3);
    } else {
        ConsoleUI::RenderScreen("БУЛАТ", "Булат смеётся: «С тобой не соскучишься! Ладно, пошли уже.»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 5);
        reduceStress(state.getPlayer(), 5);
    }
    ConsoleUI::WaitForEnter();

    // Перед экзаменом — Семён предлагает шпаргалку
    {
        ConsoleUI::RenderScreen("ПЕРЕД ЭКЗАМЕНОМ",
            "Перед аудиторией к тебе подходит Семён.\n"
            "Он оглядывается по сторонам и говорит вполголоса:\n"
            "«Псс, " + state.getPlayer().getName() + "! Держи шпору.\n"
            "Я её вчера составил. Там все формулы по графам.\n"
            "Пользоваться или нет — решай сам.»",
            {"Взять шпаргалку — пригодится",
             "Отказаться — надеяться на свои знания",
             "Попросить объяснить сложные моменты устно"},
            state.getPlayer(), ConsoleUI::GetSemenPortrait(), "Семён");
    }

    choice = ConsoleUI::ReadInt();

    if (choice == 1) {
        ConsoleUI::RenderScreen("ВЫБОР",
            "Ты быстро прячешь шпаргалку в карман.\n"
            "Семён одобрительно кивает.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Семён", 5);
        state.getPlayer().setFlag("took_cheat_sheet", true);
        loseHumanity(state.getPlayer(), 5);
    } else if (choice == 2) {
        ConsoleUI::RenderScreen("ВЫБОР",
            "Ты отказываешься. Семён пожимает плечами:\n"
            "««Дело твоё. Удачи.»»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Семён", -2);
        gainHumanity(state.getPlayer(), 5);
    } else {
        ConsoleUI::RenderScreen("ВЫБОР",
            "Семён быстро, шёпотом объясняет ключевые моменты.\n"
            "Ты чувствуешь, что стало немного понятнее.",
            {}, state.getPlayer());
        gainIntellect(state.getPlayer(), 5);
        state.getPlayer().modifyRelation("Семён", 3);
        recordNPCChoice("Семён", "helped_player", 1);
    }
    ConsoleUI::WaitForEnter();

    // Экзамен по дискретной математике
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО ДИСКРЕТНОЙ МАТЕМАТИКЕ");
    DiscreteExam discreteExam;
    int score = discreteExam.runExam(state.getPlayer());

    if (state.getPlayer().hasFlag("took_cheat_sheet") && score < GameConstants::EXAM_PASS_THRESHOLD) {
        int oldScore = score;

        ConsoleUI::RenderScreen("ШПАРГАЛКА",
            "Шпаргалка помогла! Ты подсмотрел пару формул.\n+10 баллов к результату.",
            {}, state.getPlayer());

        score = std::min(100, score + 10);
        state.getPlayer().setGrade(3, score);

        if (oldScore < GameConstants::EXAM_PASS_THRESHOLD &&
            score >= GameConstants::EXAM_PASS_THRESHOLD) {
            state.getPlayer().removeDebt();
        }

        ConsoleUI::WaitForEnter();
    }

    ConsoleUI::WaitForEnter();

    // После экзамена
    ConsoleUI::PrintHeader("ПОСЛЕ ЭКЗАМЕНА");
    std::cout << BOX_V " После экзамена ты встречаешь Аллу.\n";
    interactWithAlla();

    eventManager.tryTriggerEvent(state.getPlayer(), 4);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 4");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
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

void Game::runDay6() {
    ConsoleUI::PrintDayHeader(6, "Выходной");
    state.getPlayer().setFlag("day7_available", true);

    {
        ConsoleUI::RenderScreen("УТРО ВЫХОДНОГО",
            "Наконец-то выходной! Экзаменов нет.\n"
            "Можно выдохнуть и заняться чем угодно.\n"
            "Что будешь делать?",
            {"Отдыхать дома (спать, есть, смотреть сериалы)",
             "Пойти работать (подработка, +деньги)",
             "Пойти гулять по городу",
             "Встретиться с друзьями",
             "Позвонить Алле"},
            state.getPlayer());
    }

    int choice = ConsoleUI::ReadInt();

    switch (choice) {
    case 1:
        ConsoleUI::RenderScreen("ДОМА",
            "Ты отдыхаешь дома: спишь до обеда, смотришь\n"
            "Netflix, заказываешь пиццу. Благодать.",
            {}, state.getPlayer());
        gainEnergy(state.getPlayer(), 40);
        reduceFatigue(state.getPlayer(), 30);
        gainHunger(state.getPlayer(), 10);
        reduceStress(state.getPlayer(), 15);
        state.getPlayer().advanceTime(360);
        break;
    case 2: {
        ConsoleUI::RenderScreen("РАБОТА",
            "Ты идёшь на подработку в кофейню.\n"
            "Несколько часов за стойкой — и деньги в кармане.",
            {"Работать бариста (+300 руб)",
             "Работать курьером (+400 руб, но устанешь)"},
            state.getPlayer());

        choice = ConsoleUI::ReadInt();

        if (choice == 1) {
            gainMoney(state.getPlayer(), 300);
            gainFatigue(state.getPlayer(), 10);
            loseEnergy(state.getPlayer(), 10);
        } else {
            gainMoney(state.getPlayer(), 400);
            gainFatigue(state.getPlayer(), 20);
            loseEnergy(state.getPlayer(), 20);
        }
        state.getPlayer().advanceTime(240);
        break;
    }
    case 3:
        ConsoleUI::RenderScreen("ПРОГУЛКА",
            "Ты гуляешь по городу. Солнце, птицы, мороженое.\n"
            "Жизнь прекрасна, когда нет экзаменов.",
            {}, state.getPlayer());
        reduceStress(state.getPlayer(), 15);
        gainEnergy(state.getPlayer(), 5);
        state.getPlayer().advanceTime(120);
        break;
    case 4: {
        ConsoleUI::RenderScreen("ДРУЗЬЯ",
            "Ты встречаешься с Булатом и Семёном.\n"
            "Булат предлагает сыграть в настолки.",
            {"Согласиться — отдохнуть и повеселиться",
             "Обсуждать учёбу и стратегию"},
            state.getPlayer());

        choice = ConsoleUI::ReadInt();

        if (choice == 1) {
            reduceStress(state.getPlayer(), 20);
            state.getPlayer().modifyRelation("Булат", 5);
            state.getPlayer().modifyRelation("Семён", 3);
        } else {
            gainIntellect(state.getPlayer(), 4);
            state.getPlayer().modifyRelation("Семён", 5);
        }
        state.getPlayer().advanceTime(150);
        break;
    }
    case 5: {
        ConsoleUI::RenderScreen("ЗВОНОК АЛЛЕ",
            "Ты набираешь номер Аллы.\n"
            "Она отвечает после второго гудка:\n"
            "«Привет! А я как раз думала, что ты не позвонишь.»\n\n"
            "О чём будешь говорить?",
            {"Спросить, как у неё дела",
             "Позвать гулять",
             "Обсудить предстоящие экзамены",
             "Сказать комплимент"},
            state.getPlayer());

        choice = ConsoleUI::ReadInt();

        if (choice == 1) {
            state.getPlayer().modifyRelation("Алла", 5);
            gainRomance(state.getPlayer(), 3);
        } else if (choice == 2) {
            state.getPlayer().modifyRelation("Алла", 8);
            gainRomance(state.getPlayer(), 5);
            state.getPlayer().setFlag("invited_alla_to_walk", true);
        } else if (choice == 3) {
            state.getPlayer().modifyRelation("Алла", 3);
            gainIntellect(state.getPlayer(), 2);
        } else {
            state.getPlayer().modifyRelation("Алла", 10);
            gainRomance(state.getPlayer(), 8);
            recordNPCChoice("Алла", "compliment", 1);
        }
        ConsoleUI::RenderScreen("ТЕЛЕФОН",
            "Вы болтаете около часа. Настроение отличное!",
            {}, state.getPlayer());
        state.getPlayer().advanceTime(60);
        break;
    }
    }
    ConsoleUI::WaitForEnter();

    eventManager.tryTriggerEvent(state.getPlayer(), 6);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 6");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 7 — СВИДАНИЕ ====================

void Game::runDay7() {
    ConsoleUI::PrintDayHeader(7, "Свидание");

    if (state.getPlayer().getRelation("Алла") >= 60 && state.getPlayer().getStats().romance >= 30) {
        {
            std::string dateText =
                "Сегодня особенный день.\n"
                "Алла сама подходит к тебе после пар:\n"
                "«Привет! Я подумала... может, сходим\n"
                "куда-нибудь сегодня?»\n"
                "Твоё сердце пропускает удар.";
            ConsoleUI::RenderScreen("ПРИГЛАШЕНИЕ", dateText,
                {"Согласиться — конечно, с удовольствием!",
                 "Сказать, что нужно готовиться к экзамену"},
                state.getPlayer(), ConsoleUI::GetAllaPortrait(), "Алла");
        }

        int choice = ConsoleUI::ReadInt();

        if (choice == 2) {
            ConsoleUI::RenderScreen("ОТКАЗ",
                "Алла разочарованно вздыхает:\n"
                "«Ну... ладно. Понимаю. В другой раз.»\n"
                "Ты чувствуешь, что упустил что-то важное.",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", -10);
            loseRomance(state.getPlayer(), 5);
            recordNPCChoice("Алла", "refused_date", 1);
            ConsoleUI::WaitForEnter();
            goto day7_skip_date;
        }

        // Куда пойти?
        ConsoleUI::RenderScreen("КУДА ПОЙТИ?",
            "Алла ждёт твоего предложения. Куда пойдёте?",
            {"В кафе — уютная атмосфера, вкусные десерты",
             "В парк — прогулка на свежем воздухе",
             "В кино — классика свиданий"},
            state.getPlayer(), ConsoleUI::GetAllaPortrait(), "Алла");

        choice = ConsoleUI::ReadInt();

        std::string dateResult;
        int moneyCost = 0;
        if (choice == 1) {
            dateResult =
                "Вы сидите в уютном кафе при свечах.\n"
                "Алла заказывает чизкейк, ты — капучино.\n"
                "Вы говорите обо всём на свете.\n"
                "Алла рассказывает, что мечтает путешествовать.";
            moneyCost = 400;
        } else if (choice == 2) {
            dateResult =
                "Вы гуляете по центральному парку.\n"
                "Кормите уток в пруду, катаетесь на колесе обозрения.\n"
                "На закате Алла кладёт голову тебе на плечо.";
            moneyCost = 200;
        } else {
            dateResult =
                "Вы идёте на новый фильм.\n"
                "В зале темно, и Алла берёт тебя за руку.\n"
                "Ты больше смотришь на неё, чем на экран.";
            moneyCost = 300;
        }

        int actualDateCost = scaledCost(state.getPlayer(), moneyCost);

        if (state.getPlayer().getStats().money >= actualDateCost) {
            spendMoney(state.getPlayer(), moneyCost);
            ConsoleUI::RenderScreen("СВИДАНИЕ", dateResult, {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 15);
            gainRomance(state.getPlayer(), 12);
            reduceStress(state.getPlayer(), 15);
        } else {
            ConsoleUI::RenderScreen("СВИДАНИЕ",
                "У тебя не хватает денег на запланированное.\n"
                "Приходится идти просто гулять.\n"
                "Алла говорит, что ей всё равно нравится проводить с тобой время.",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 8);
            gainRomance(state.getPlayer(), 5);
        }
        ConsoleUI::WaitForEnter();

        // Подарок
        if (state.getPlayer().hasFlag("has_flowers")) {
            ConsoleUI::RenderScreen("ЦВЕТЫ",
                "Ты даришь Алле цветы, которые купил заранее.\n"
                "Она ахает: «Ой, какие красивые! Ты запомнил,\n"
                "что я люблю ромашки? Спасибо!»\n"
                "Она обнимает тебя. Ты чувствуешь тепло.",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 15);
            gainRomance(state.getPlayer(), 10);
        }
        ConsoleUI::WaitForEnter();

        // Дождь
        ConsoleUI::RenderScreen("ДОЖДЬ",
            "Когда вы выходите, начинается дождь.\n"
            "У тебя есть зонт.",
            {"Предложить Алле укрыться под одним зонтом",
             "Отдать зонт Алле, промокнуть самому",
             "Сказать: «Романтика, правда?» и улыбнуться"},
            state.getPlayer());

        choice = ConsoleUI::ReadInt();

        switch (choice) {
        case 1:
            ConsoleUI::RenderScreen("ДОЖДЬ",
                "Вы идёте под одним зонтом, прижавшись друг к другу.\n"
                "Алла смеётся: «Уютно, хоть и мокро.»",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 8);
            gainRomance(state.getPlayer(), 5);
            break;
        case 2:
            ConsoleUI::RenderScreen("ДОЖДЬ",
                "Алла укрывается зонтом, а ты мокнешь под дождём.\n"
                "Она смотрит на тебя с нежностью: «Ты такой заботливый...»",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 12);
            gainRomance(state.getPlayer(), 8);
            loseHealth(state.getPlayer(), 5);
            break;
        case 3:
            ConsoleUI::RenderScreen("ДОЖДЬ",
                "Алла смеётся: «С тобой даже дождь — приключение!»\n"
                "Вы оба промокаете, но настроение отличное.",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 10);
            gainRomance(state.getPlayer(), 7);
            reduceStress(state.getPlayer(), 10);
            break;
        }
        ConsoleUI::WaitForEnter();

        // Итог свидания
        ConsoleUI::RenderScreen("ИТОГ СВИДАНИЯ",
            "Вы прощаетесь у её дома.\n"
            "Алла смотрит тебе в глаза:\n"
            "«Спасибо за этот день. Он был чудесным.»",
            {}, state.getPlayer());

        if (state.getPlayer().getStats().romance >= 65 &&
            state.getPlayer().getRelation("Алла") >= 75) {
            ConsoleUI::RenderScreen("МОМЕНТ",
                "Она привстаёт на цыпочки и целует тебя в щёку.\n"
                "А потом быстро забегает в подъезд, улыбаясь.\n"
                "Ты стоишь под дождём и улыбаешься как дурак.\n"
                "Это было прекрасно.",
                {}, state.getPlayer());
            state.getPlayer().setFlag("romantic_ending", true);
        } else {
            ConsoleUI::RenderScreen("ПРОЩАНИЕ",
                "Вы тепло прощаетесь. Ты чувствуешь,\n"
                "что между вами возникла особая связь.",
                {}, state.getPlayer());
        }
        state.getPlayer().setFlag("day7_done", true);
        ConsoleUI::WaitForEnter();
    } else {
        {
            ConsoleUI::RenderScreen("ДЕНЬ БЕЗ СВИДАНИЯ",
                "Сегодня никто не приглашает тебя на свидание.\n"
                "Отношения с Аллой ещё недостаточно тёплые.\n"
                "Может быть, в другой жизни...\n\n"
                "Чем займёшься?",
                {"Готовиться к последним экзаменам",
                 "Пойти гулять с Булатом",
                 "Просто отдыхать"},
                state.getPlayer());
        }

        int choice = ConsoleUI::ReadInt();

        switch (choice) {
        case 1:
            ConsoleUI::RenderScreen("ПОДГОТОВКА", "Ты зубришь комп. сети.", {}, state.getPlayer());
            gainIntellect(state.getPlayer(), 8);
            gainFatigue(state.getPlayer(), 10);
            break;
        case 2:
            interactWithBulat();
            break;
        case 3:
            ConsoleUI::RenderScreen("ОТДЫХ", "Ты отдыхаешь весь день.", {}, state.getPlayer());
            gainEnergy(state.getPlayer(), 20);
            reduceStress(state.getPlayer(), 10);
            break;
        }
        ConsoleUI::WaitForEnter();
    }

    day7_skip_date:
    eventManager.tryTriggerEvent(state.getPlayer(), 7);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 7");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
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
            "Позаниматься"
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
    ConsoleUI::WaitForEnter();
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
                state.getPlayer().setFlag("gave_flowers_to_alla", true);
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

    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}

void Game::interactWithBulat() {
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

    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}

void Game::interactWithSemen() {
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

    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}

void Game::interactWithArtem() {
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

    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}
