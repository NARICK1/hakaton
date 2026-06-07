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
#include <iostream>
#include <sstream>
#include <algorithm>

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

void Game::initGame() {
    state = GameState();
    state.getPlayer() = Player("Тимур");
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

    ConsoleUI::RenderScreen(
        title,
        text,
        {
            "Поесть дома (-" + std::to_string(GameConstants::EAT_MONEY_COST) +
                " руб, +" + std::to_string(GameConstants::EAT_HUNGER_RESTORE) +
                " сытости)",
            "Не есть"
        },
        player,
        "",
        "",
        "Дом"
    );

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

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
        } else if (stats.money < GameConstants::EAT_MONEY_COST) {
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
    ConsoleUI::PrintDayHeader(1, "История");
    std::cout << "\nПервый день. Экзамен по истории.\n";
    std::cout << "Ты достаточно времени провел за подготовкой, однако не все удалось запомнить.\n»;
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();
    int examResult = 0;
    int teacherRelation = state.getPlayer().getRelation("Преподаватель истории");
    if (teacherRelation == 0) teacherRelation = 50;

    // Показываем параметры
    ConsoleUI::PrintHeader("ПОДГОТОВКА К ЭКЗАМЕНУ");
    std::cout << "\nТвои параметры:\n";
    std::cout << "• Интеллект: " << stats.intellect << "/100\n";
    std::cout << "• Отношения с Аллой: " << state.getPlayer().getRelation("Алла") << "/100\n\n";

    ConsoleUI::WaitForEnter();

    // Выбор билета
    ConsoleUI::PrintHeader("ВЫБОР БИЛЕТА");
    std::cout << "Преподаватель раздает билеты. Какой вытащишь?\n\n";
    std::cout << "1. Билет слева\n";
    std::cout << "2. Билет посередине\n";
    std::cout << "3. Билет справа\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    // Билет оказывается незнакомым
    std::cout << "\nТы открываешь билет... Он оказывается незнакомым!\n";
    ConsoleUI::WaitForEnter();

    // Выбор действия
    ConsoleUI::PrintHeader("ЧТО ДЕЛАТЬ?");
    std::cout << "Нужно как-то его заполнить.\n\n";
    std::cout << "1. Попросить помощь у Аллы\n";
    std::cout << "2. Дождаться, пока преподаватель выйдет, и посмотреть в телефоне ответы\n»;
    std::cout << "3. Отвечать самостоятельно\n»;

    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    int allaRelation = state.getPlayer().getRelation("Алла");

    // ========== 1.1.1 ПОМОЩЬ АЛЛЫ ==========
    if (choice == 1) {
        std::cout << "\n--- Помощь Аллы ---\n";

        if (allaRelation >= 50) {
            std::cout << "Алла пишет на черновике решение главному герою.\n\n";
            std::cout << "Что дальше?\n";
            std::cout << "1. Подождать еще час\n";
            std::cout << "2. Сдать сейчас\n";

            std::cin >> choice;
            std::cin.ignore(10000, '\n');

            if (choice == 1) {
                // 1.1.1.1.1 Подождать час
                std::cout << "\nТы ждешь час.  Потом аккуратно с умным видом сдаешь работу. Все прошло хорошо.\n»;
                examResult = 4;
                std::cout << "\nТвоя оценка: 4\n";
            } else {
                // 1.1.1.1.2 Сдать сейчас
                std::cout << "\nТы сдаешь билет. Преподаватель понимает, что так быстро ты не мог его заполнить.\n";
                std::cout << "Он задает дополнительные вопросы. Ты молчишь и не можешь на них ответить.\n";
                examResult = 2;
                std::cout << "\nТвоя оценка: 2\n";
                state.getPlayer().addDebt(1);
            }
        } else {
            std::cout << "Алла не помогает. Отношения недостаточно высокие.\n";
            std::cout << "Приходится действовать самостоятельно.\n»;
            
            // Самостоятельный ответ
            goto SELF_ANSWER;
        }
    }
    // ========== 1.1.2 ДОЖДАТЬСЯ ПРЕПОДАВАТЕЛЯ ==========
    else if (choice == 2) {
        std::cout << "\n--- Ожидание преподавателя ---\n";
        
        bool examEnded = false;
        int waitHours = 0;

        while (!examEnded && waitHours < 4) {
            std::cout << "\nПроходит час. Преподаватель сидит на месте.\n";
            std::cout << "1. Подождать еще час\n";
            std::cout << "2. Выйти в туалет\n";

            std::cin >> choice;
            std::cin.ignore(10000, '\n');

            if (choice == 1) {
                waitHours++;
                if (waitHours >= 4) {
                    // 1.1.2.1.1.1.1 Подождать еще час (время вышло)
                    std::cout << "\nВремя на экзамен заканчивается. Ты идешь к преподавателю с пустым билетом.\n";
                    examResult = 2;
                    std::cout << "\nТвоя оценка: 2\n";
                    state.getPlayer().addDebt(1);
                    examEnded = true;
                }
            } else if (choice == 2) {
                // 1.1.2.1.1.1.2 Выйти в туалет
                std::cout << "\nТы выходишь в туалет. Встречаешь друга\n\n";
                std::cout << "1. Попросить помощь\n";
                std::cout << "2. Умыть лицо\n";

                std::cin >> choice;
                std::cin.ignore(10000, '\n');

                if (choice == 1) {
                    // 1.1.2.1.1.1.1.1 Попросить помощь
                    std::cout << "\nДруг полностью рассказывает билет. Ты запоминаешь ответы.\n";
                    std::cout << "Возвращаешься в аудиторию, пишешь билет и идешь сдавать.\n";
                    std::cout << "Преподаватель видит, что ты ответил уверенно.\n";
                    examResult = 4;
                    std::cout << "\nТвоя оценка: 4\n";
                } else {
                    // 1.1.2.1.1.1.1.2 Умыть лицо
                    std::cout << "\nТы умываешь лицо холодной водой и идешь обратно в аудиторию.\n";
                    std::cout << "Твой незаполненный билет лежит на месте. Преподаватель зовет сдавать.\n";
                    std::cout << "Ты пытаешься отвечать на вопросы...\n";
                    examResult = 2;
                    std::cout << "\nТвоя оценка: 2\n";
                    state.getPlayer().addDebt(1);
                }
                examEnded = true;
            }
        }
    }
    // ========== 1.1.3 ОТВЕЧАТЬ САМОСТОЯТЕЛЬНО ==========
    else {
        SELF_ANSWER:
        std::cout << "\n--- Самостоятельный ответ ---\n";
        std::cout << "Преподаватель задает вопросы, ты отвечаешь.\n\n";

        struct Question {
            const char* text;
            int correctYear;
        };

        Question questions[5] = {
            {"В каком году была Куликовская битва?", 1380},
            {"В каком году произошло Крещение Руси?", 988},
            {"В каком году началась Отечественная война 1812 года?", 1812},
            {"В каком году произошла Октябрьская революция?", 1917},
            {"В каком году был первый полёт человека в космос?", 1961}
        };

        int correctAnswers = 0;

        for (int i = 0; i < 5; i++) {
            std::cout << "Вопрос " << (i + 1) << ": " << questions[i].text << "\n";
            std::cout << "Твой ответ (год): ";
            
            int answer;
            std::cin >> answer;
            std::cin.ignore(10000, '\n');

            if (answer == questions[i].correctYear) {
                std::cout << "Правильно!\n\n";
                correctAnswers++;
            } else {
                std::cout << "Неправильно. Правильный ответ: " << questions[i].correctYear << "\n";
                
                // 1.1.3 Нюанс: спор с преподавателем
                std::cout << "Будешь спорить? (1 - да, 2 - нет): ";
                int dispute;
                std::cin >> dispute;
                std::cin.ignore(10000, '\n');
                
                if (dispute == 1) {
                    std::cout << "Ты начинаешь спорить с преподавателем.\n";
                    teacherRelation -= 15;
                    state.getPlayer().modifyRelation("Преподаватель истории", -15);
                    std::cout << "Отношение с преподавателем ухудшилось!\n";
                }
                std::cout << "\n";
            }
        }

        if (correctAnswers >= 5) examResult = 5;
        else if (correctAnswers >= 4) examResult = 4;
        else if (correctAnswers >= 3) examResult = 3;
        else examResult = 2;

        std::cout << "Правильных ответов: " << correctAnswers << " из 5\n";
        std::cout << "\nТвоя оценка: " << examResult << "\n";

        if (examResult == 2) {
            state.getPlayer().addDebt(1);
        }
    }

    // Сохраняем результат
    state.getPlayer().setGrade(1, examResult);

    // ========== КОНЕЦ ДНЯ ==========
    ConsoleUI::PrintHeader("ИТОГ ДНЯ");
    std::cout << "\nВаша оценка: " << examResult << "\n";
    std::cout << "Экзамен сдан. Теперь домой спать.\n";

    // Обновление характеристик в конце дня
    stats.fatigue = std::min(stats.fatigue + 15, 100);
    stats.hunger = std::min(stats.hunger + 10, 100);
    if (examResult == 2) {
        stats.stress = std::min(stats.stress + 15, 100);
    }
    stats.clampAll();

    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}


// ==================== ДЕНЬ 2 — ЯИМП ====================
void Game::runDay2() {
    ConsoleUI::PrintDayHeader(2, "Языки и методы программирования");
    std::cout << "\nВторой день. Экзамен по ЯиМП.\n";
    std::cout << "Ты плохо подготовлен к экзамену.\n»;
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();
    int examResult = 0;

    // Показываем параметры
    ConsoleUI::PrintHeader("ПОДГОТОВКА К ЭКЗАМЕНУ");
    std::cout << "\nТвои параметры:\n";
    std::cout << "• Интеллект: " << stats.intellect << "/100\n";
    std::cout << "• Деньги: " << stats.money << " руб.\n\n";

    ConsoleUI::WaitForEnter();

    // 2.1 Выбор билета
    ConsoleUI::PrintHeader("ВЫБОР БИЛЕТА");
    std::cout << "Преподаватель предлагает выбрать билет. Один из 10 билет - счастливый.\n»;
    std::cout << "Выбери номер билета (1-10):\n\n";
    std::cout << "1  2  3  4  5\n";
    std::cout << "6  7  8  9  10\n";

    int ticket;
    std::cin >> ticket;
    std::cin.ignore(10000, '\n');

    // 2.1.1 Проверка на счастливый билет (билет №7)
    const int LUCKY_TICKET = 7;

    if (ticket == LUCKY_TICKET) {
        // 2.1.1.1 Счастливый билет
        std::cout << "\n✨ ТЫ ВЫТЯНУЛ СЧАСТЛИВЫЙ БИЛЕТ! ✨\n";
        std::cout << "Ты отвечаешь на вопросы без особых затруднений.\n";
        std::cout << "Преподаватель ставит оценку 5.\n";
        examResult = 5;
        stats.stress = std::max(stats.stress - 15, 0);
    } else {
        // 2.1.1.2 Обычный билет
        std::cout << "\nТы вытянул обычный билет.\n";
        std::cout << "Несмотря на отсутствие подготовки, вопросы оказываются несложными.\n";
        std::cout << "Преподаватель ставит оценку 4.\n";
        examResult = 4;
        stats.intellect = std::min(stats.intellect + 2, 100);
    }

    // Сохраняем результат
    state.getPlayer().setGrade(2, examResult);

    ConsoleUI::WaitForEnter();

    // 2.2 После экзамена главный герой идет в столовую
    ConsoleUI::PrintHeader("ПОСЛЕ ЭКЗАМЕНА");
    std::cout << "\nТы идёшь в столовую.\n";
    ConsoleUI::WaitForEnter();

    // 2.2.1 Встреча с Артемом
    ConsoleUI::PrintHeader("ВСТРЕЧА С АРТЁМОМ");
    std::cout << "Навстречу идёт Артём. Он явно не в настроении.\n";
    std::cout << "Ты пытаешься пройти мимо, но Артём преграждает путь.\n\n";
    std::cout << "Что делать?\n\n";
    std::cout << "1. Убежать\n";
    std::cout << "2. Попытаться постоять за себя\n";
    std::cout << "3. Молча отдать деньги\n";

    int fightChoice;
    std::cin >> fightChoice;
    std::cin.ignore(10000, '\n');

    switch (fightChoice) {
        case 1: {
            // 2.2.1.1 Убежать
            std::cout << "\nТы убегаешь, привлекая внимание окружающих студентов.\n";
            std::cout << "Ты чувствуешь себя униженным, но избегаешь конфликта.\n";
            std::cout << "Потеряно много энергии.\n";
            stats.fatigue = std::min(stats.fatigue + 35, 100);
            stats.stress = std::min(stats.stress + 20, 100);
            stats.humanity = std::max(stats.humanity - 10, 0);
            break;
        }
        case 2: {
            // 2.2.1.2 Попытаться постоять за себя
            std::cout << "\nАртём начинает издеваться над тобой и толкает тебя.\n";
            std::cout << "Конфликт быстро заканчивается не в твою пользу.\n";
            std::cout << "Потеряно часть здоровья и небольшое количество энергии.\n";
            stats.health = std::max(stats.health - 20, 0);
            stats.fatigue = std::min(stats.fatigue + 15, 100);
            stats.stress = std::min(stats.stress + 25, 100);
            state.getPlayer().modifyRelation("Артём", -15);
            break;
        }
        case 3: {
            // 2.2.1.3 Молча отдать деньги (четверть от имеющихся)
            int moneyToGive = stats.money / 4;
            if (moneyToGive > 0) {
                stats.money -= moneyToGive;
                std::cout << "\nТы молча отдаёшь " << moneyToGive << " рублей (четверть твоих денег).\n";
                std::cout << "Артём теряет интерес и уходит.\n";
                std::cout << "Здоровье и энергия не изменились.\n";
            } else {
                std::cout << "\nУ тебя нет денег. Артём злится и толкает тебя.\n";
                stats.health = std::max(stats.health - 15, 0);
                stats.fatigue = std::min(stats.fatigue + 20, 100);
                stats.stress = std::min(stats.stress + 20, 100);
            }
            break;
        }
        default:
            std::cout << "\nТы стоишь в ступоре. Артём теряет интерес и уходит.\n";
            break;
    }

    ConsoleUI::WaitForEnter();

    // 2.3 Конец дня
    ConsoleUI::PrintHeader("ИТОГ ДНЯ 2");
    std::cout << "\nТвоя оценка за экзамен: " << examResult << "\n";
    std::cout << "После произошедшего ты добираешься до столовой. Плотно поев, ты восстанавливаешься.\n»;
    std::cout << "Второй день заканчивается. Теперь домой спать.\n";

    // Сон восстанавливает силы
    stats.energy = GameConstants::MAX_STAT;
    stats.fatigue = 0;
    stats.hunger = std::min(stats.hunger + 10, 100);
    if (examResult == 2) {
        stats.stress = std::min(stats.stress + 15, 100);
    } else {
        stats.stress = std::max(stats.stress - 5, 0);
    }
    stats.clampAll();

    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

  
       
   
// ==================== ДЕНЬ 3 — ПОДГОТОВКА ====================
void Game::runDay3() {
    ConsoleUI::PrintDayHeader(3, "Подготовка");
    std::cout << "\nТретий день. Сегодня экзаменов нет.\n";
    std::cout << "В расписании две пары: физкультура и 3D-моделирование.\n";
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();

    // Показываем параметры
    ConsoleUI::PrintHeader("ПАРАМЕТРЫ ПЕРЕД НАЧАЛОМ ДНЯ");
    std::cout << "\nТвои параметры:\n";
    std::cout << "• Энергия: " << stats.energy << "/100\n";
    std::cout << "• Отношения с Аллой: " << state.getPlayer().getRelation("Алла") << "/100\n";
    std::cout << "• Отношения с Семёном: " << state.getPlayer().getRelation("Семён") << "/100\n\n";

    ConsoleUI::WaitForEnter();

    // 3.1 Главный герой приходит в университет
    ConsoleUI::PrintHeader("ПО ДОРОГЕ НА ФИЗКУЛЬТУРУ");
    std::cout << "\nТы идёшь на физкультуру. Навстречу попадается Семён.\n";
    std::cout << "Семён: «Привет, Тимур! Пойдёшь после пары со мной обедать? Я угощаю)»\n\n";
    std::cout << "Твой ответ:\n";
    std::cout << "1. Да, дам ответ после физкультуры\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');
    
    // 3.1.1.1 Главный герой говорит, что даст ответ после физкультуры
    std::cout << "\nТы: «Да, дам ответ после физкультуры»\n";
    std::cout << "Семён: «Хорошо, жду!»\n";

    ConsoleUI::WaitForEnter();

    // 3.2 Начинается физкультура
    ConsoleUI::PrintHeader("ФИЗКУЛЬТУРА");
    std::cout << "\nНачинается физкультура. Ты активно участвуешь в занятии.\n";
    std::cout << "Бег, отжимания, подтягивания...\n";
    
    // 3.2.1 После пары чувствует усталость
    std::cout << "\nПосле пары ты чувствуешь сильную усталость.\n";
    stats.energy = std::max(stats.energy - 25, 0);
    stats.fatigue = std::min(stats.fatigue + 20, 100);
    std::cout << "Энергия -25, усталость +20\n";

    ConsoleUI::WaitForEnter();

    // 3.3 Между парами обеденный перерыв
    ConsoleUI::PrintHeader("ОБЕДЕННЫЙ ПЕРЕРЫВ");
    std::cout << "\nНаступает обеденный перерыв.\n";
    std::cout << "Семён подходит к тебе: «Ну что, идём?»\n\n";
    std::cout << "Твой выбор:\n";
    std::cout << "1. Пойти с Семёном в столовую\n";
    std::cout << "2. Отказаться: «Спасибо, Семён, что позвал. Но я хочу поесть в одиночестве»\n";

    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice == 1) {
        // 3.3.1 Пойти с Семёном
        std::cout << "\nВы идёте в столовую вместе. Семён угощает тебя обедом.\n";
        std::cout << "Вы мило беседуете за едой.\n";
        stats.energy = std::min(stats.energy + 20, 100);
        state.getPlayer().modifyRelation("Семён", 10);
        std::cout << "\nЭнергия +20, отношения с Семёном улучшились (+10)\n";
    } else {
        // 3.3.2 Отказать Семёну
        std::cout << "\nТы: «Спасибо, Семён, что позвал. Но я хочу поесть в одиночестве»\n";
        std::cout << "Семён: «Ну ладно...» - он выглядит разочарованным.\n";
        std::cout << "Ты идёшь есть один в столовую.\n";
        stats.energy = std::min(stats.energy + 15, 100);
        state.getPlayer().modifyRelation("Семён", -8);
        std::cout << "\nЭнергия +15, отношения с Семёном ухудшились (-8)\n";
    }

    ConsoleUI::WaitForEnter();

    // 3.4 Пара по 3D-моделированию
    ConsoleUI::PrintHeader("3D-МОДЕЛИРОВАНИЕ");
    std::cout << "\nНачинается пара по 3D-моделированию.\n";
    std::cout << "Ты заходишь в аудиторию и замечаешь Аллу.\n";
    std::cout << "Ты решаешь подсесть к ней.\n\n";
    std::cout << "Что скажешь?\n";
    std::cout << "1. Начать милый разговор\n";
    std::cout << "2. Спросить про учёбу\n";

    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice == 1) {
        // 3.4.1.1 Начать милый разговор
        std::cout << "\nТы: «Привет, Алла, отлично выглядишь! Как ты провела прошлые выходные?»\n";
        std::cout << "Алла засмущалась: «Спасибо, Тимур, мне очень приятно».\n";
        std::cout << "Алла: «Я готовилась к экзамену по дискретной математике, а ты чем занимался?»\n";
        std::cout << "Вы начинаете милую беседу.\n";
        
        state.getPlayer().modifyRelation("Алла", 12);
        stats.energy = std::min(stats.energy + 10, 100);
        stats.romance = std::min(stats.romance + 8, 100);
        std::cout << "\nОтношения с Аллой улучшились (+12), энергия +10, романтика +8\n";
    } else {
        // 3.4.1.2 Спросить про учёбу
        std::cout << "\nТы: «Привет, как идёт твоя подготовка к экзаменам?»\n";
        std::cout << "Алла сухо отвечает: «Хорошо».\n";
        std::cout << "Она не продолжает беседу и отворачивается.\n";
        std::cout << "Алла разочарована, что ты интересуешься только её учёбой.\n";
        
        state.getPlayer().modifyRelation("Алла", -10);
        stats.stress = std::min(stats.stress + 5, 100);
        std::cout << "\nОтношения с Аллой ухудшились (-10), стресс +5\n";
    }

    ConsoleUI::WaitForEnter();

    // 3.5 Конец дня
    ConsoleUI::PrintHeader("ИТОГ ДНЯ 3");
    std::cout << "\nПара заканчивается. Ты возвращаешься домой.\n";
    std::cout << "Ты готовишься к экзамену по дискретной математике.\n";
    
    // Интеллект увеличивается
    stats.intellect = std::min(stats.intellect + 8, 100);
    std::cout << "Интеллект +8\n";
    
    std::cout << "Третий день заканчивается. Ты ложишься спать.\n";

    // Сон восстанавливает силы
    stats.energy = GameConstants::MAX_STAT;
    stats.fatigue = 0;
    stats.hunger = std::min(stats.hunger + 10, 100);
    stats.stress = std::max(stats.stress - 5, 0);
    stats.clampAll();

    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

// ==================== ДЕНЬ 4 — ДИСКРЕТНАЯ МАТЕМАТИКА ====================

void Game::runDay4() {
    ConsoleUI::PrintDayHeader(4, "Дискретная математика");
    std::cout << "\nЧетвертый день. Экзамен по дискретной математике.\n";
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();
    int examResult = 0;
    int helpedBulat = 0; // 0 - не помогал, 1 - помогал
    int ticketDifficulty = 0; // 0 - простой, 1 - сложный

    // Показываем параметры
    ConsoleUI::PrintHeader("ПАРАМЕТРЫ ПЕРЕД ЭКЗАМЕНОМ");
    std::cout << "\nТвои параметры:\n";
    std::cout << "• Интеллект: " << stats.intellect << "/100\n";
    std::cout << "• Усталость: " << stats.fatigue << "/100\n";
    std::cout << "• Отношения с Аллой: " << state.getPlayer().getRelation("Алла") << "/100\n";
    std::cout << "• Отношения с Булатом: " << state.getPlayer().getRelation("Булат") << "/100\n\n";

    ConsoleUI::WaitForEnter();

    // 4.1 Ночь перед экзаменом
    ConsoleUI::PrintHeader("НОЧЬ ПЕРЕД ЭКЗАМЕНОМ");
    std::cout << "Ты не можешь уснуть. Мысли об экзамене не дают покоя.\n\n";
    std::cout << "Что будешь делать?\n";
    std::cout << "1. Лечь спать\n";
    std::cout << "2. Играть всю ночь\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice == 1) {
        // 4.1.1 Лечь спать
        std::cout << "\nТы решаешь лечь спать и хорошо высыпаешься перед экзаменом.\n";
        stats.fatigue = std::max(stats.fatigue - 40, 0);
        stats.energy = std::min(stats.energy + 30, 100);
        std::cout << "Усталость -40, энергия +30\n";
    } else {
        // 4.1.2 Играть всю ночь
        std::cout << "\nТы решаешь отвлечься от мыслей об экзамене и играешь всю ночь.\n";
        stats.stress = std::max(stats.stress - 30, 0);
        stats.fatigue = std::min(stats.fatigue + 35, 100);
        stats.energy = std::max(stats.energy - 20, 0);
        std::cout << "Стресс полностью исчез! (-30), но усталость +35, энергия -20\n";
    }

    ConsoleUI::WaitForEnter();

    // 4.2 Главный герой приходит на экзамен
    ConsoleUI::PrintHeader("ПЕРЕД ЭКЗАМЕНОМ");
    std::cout << "\nТы приходишь на экзамен по дискретной математике.\n";
    std::cout << "Перед входом в аудиторию замечаешь Булата. Он выглядит очень встревоженным.\n";
    std::cout << "Булат: «Брат, если я завалю, меня отчислят».\n\n";
    std::cout << "Твои действия:\n";
    std::cout << "1. Помочь Булату\n";
    std::cout << "2. Не помогать и поискать Аллу\n";

    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice == 1) {
        // 4.2.1 Помочь Булату
        std::cout << "\nТы решаешь помочь Булату. Вы вместе повторяете основные темы.\n";
        std::cout << "Вы разбираете сложные задания. Булат становится увереннее.\n";
        helpedBulat = 1;
        state.getPlayer().modifyRelation("Булат", 15);
        stats.fatigue = std::min(stats.fatigue + 15, 100);
        std::cout << "\nОтношения с Булатом улучшились (+15)!\n";
        std::cout << "Но ты потратил время и не успел поговорить с Аллой перед экзаменом.\n";
        std::cout << "Усталость +15\n";
    } else {
        // 4.2.2 Не помогать и поискать Аллу
        std::cout << "\nТы решаешь не помогать Булату и идёшь искать Аллу.\n";
        std::cout << "Булат остаётся один со своими переживаниями.\n";
        std::cout << "Ты находишь Аллу перед экзаменом. Вы успеваете немного пообщаться.\n";
        state.getPlayer().modifyRelation("Алла", 8);
        stats.romance = std::min(stats.romance + 5, 100);
        state.getPlayer().modifyRelation("Булат", -10);
        std::cout << "\nОтношения с Аллой улучшились (+8)!\n";
        std::cout << "Романтика +5!\n";
        std::cout << "Отношения с Булатом ухудшились (-10).\n";
    }

    ConsoleUI::WaitForEnter();

    // 4.3 Экзамен по дискретной математике
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО ДИСКРЕТНОЙ МАТЕМАТИКЕ");
    
    // Определяем сложность билета (рандом)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    ticketDifficulty = dis(gen);
    
    if (ticketDifficulty == 0) {
        std::cout << "\nТы тянешь билет... Попался ПРОСТОЙ билет!\n\n";
        std::cout << "Что будешь делать?\n";
        std::cout << "1. Решать самостоятельно\n";
        std::cout << "2. Спросить помощь у Булата\n";
        
        std::cin >> choice;
        std::cin.ignore(10000, '\n');
        
        if (choice == 1) {
            // 4.3.1.1.1 Решать самостоятельно
            if (stats.fatigue <= 50) {
                std::cout << "\nТвоя усталость низкая. Ты успешно решаешь все задания.\n";
                examResult = 4;
                std::cout << "Оценка: 4\n";
            } else {
                std::cout << "\nТвоя усталость высокая. Ты допускаешь ошибки в заданиях.\n";
                examResult = 3;
                std::cout << "Оценка: 3\n";
            }
        } else {
            // 4.3.1.1.2 Спросить помощь у Булата
            int bulatRelation = state.getPlayer().getRelation("Булат");
            if (helpedBulat == 1 || bulatRelation >= 60) {
                std::cout << "\nБулат с радостью помогает тебе и подсказывает решения.\n";
                examResult = 4;
                std::cout << "Оценка: 4\n";
            } else {
                std::cout << "\nБулат отказывается помогать. Отношения с ним плохие.\n";
                std::cout << "Ты продолжаешь решать сам.\n";
                if (stats.fatigue <= 50) {
                    examResult = 3;
                    std::cout << "Оценка: 3\n";
                } else {
                    examResult = 2;
                    std::cout << "Оценка: 2\n";
                }
            }
        }
    } else {
        std::cout << "\nТы тянешь билет... Попался СЛОЖНЫЙ билет!\n";
        std::cout << "Ты понимаешь только часть заданий.\n\n";
        std::cout << "Что будешь делать?\n";
        std::cout << "1. Попытаться решить самостоятельно\n";
        std::cout << "2. Попросить помощь у Булата\n";
        
        std::cin >> choice;
        std::cin.ignore(10000, '\n');
        
        if (choice == 1) {
            // 4.3.1.2.1.1 Попытаться решить самостоятельно
            if (stats.fatigue <= 50) {
                std::cout << "\nНесмотря на усталость, ты вспоминаешь часть материала.\n";
                examResult = 3;
                std::cout << "Оценка: 3\n";
            } else {
                std::cout << "\nТвоя усталость высокая. Ты не справляешься с заданиями.\n";
                examResult = 2;
                std::cout << "Оценка: 2\n";
            }
        } else {
            // 4.3.1.2.1.2 Попросить помощь Булата
            int bulatRelation = state.getPlayer().getRelation("Булат");
            if (helpedBulat == 1 || bulatRelation >= 60) {
                std::cout << "\nБулат помогает тебе разобраться с одним из заданий.\n";
                examResult = 3;
                std::cout << "Оценка: 3\n";
            } else {
                std::cout << "\nБулат игнорирует твою просьбу. Отношения плохие.\n";
                examResult = 2;
                std::cout << "Оценка: 2\n";
            }
        }
    }
    
    ConsoleUI::WaitForEnter();
    
    // 4.4 Сдача преподавателю
    ConsoleUI::PrintHeader("РЕЗУЛЬТАТ ЭКЗАМЕНА");
    
    if (examResult == 4) {
        std::cout << "\nПреподаватель доволен твоим ответом.\n";
        std::cout << "Экзамен успешно сдан! Оценка: 4\n";
        stats.stress = std::max(stats.stress - 15, 0);
        stats.romance = std::min(stats.romance + 3, 100);
        std::cout << "Романтика +3\n";
    } else if (examResult == 3) {
        std::cout << "\nПреподаватель отмечает пробелы в знаниях,\n";
        std::cout << "но ставит удовлетворительную оценку. Оценка: 3\n";
        stats.stress = std::min(stats.stress + 5, 100);
    } else {
        std::cout << "\nПреподаватель сообщает о неудовлетворительном результате.\n";
        std::cout << "Экзамен завален! Оценка: 2\n";
        state.getPlayer().addDebt(1);
        stats.stress = std::min(stats.stress + 25, 100);
        stats.romance = std::max(stats.romance - 5, 0);
        std::cout << "Романтика -5\n";
    }
    
    // Сохраняем результат
    state.getPlayer().setGrade(4, examResult);
    
    ConsoleUI::WaitForEnter();
    
    // 4.5 Усталость и энергия после экзамена
    ConsoleUI::PrintHeader("ПОСЛЕ ЭКЗАМЕНА");
    std::cout << "\nЭкзамен закончен. Ты чувствуешь усталость.\n";
    stats.fatigue = std::min(stats.fatigue + 20, 100);
    stats.energy = std::max(stats.energy - 15, 0);
    std::cout << "Усталость +20, энергия -15\n";
    
    ConsoleUI::WaitForEnter();
    
    // 4.6 Столовая и возвращение домой
    ConsoleUI::PrintHeader("СТОЛОВАЯ");
    std::cout << "\nТы идёшь в столовую.\n";
    stats.hunger = std::max(stats.hunger - 30, 0);
    stats.energy = std::min(stats.energy + 20, 100);
    std::cout << "Ты поел. Голод -30, энергия +20\n";
    
    ConsoleUI::WaitForEnter();
    
    // 4.7 Конец дня
    ConsoleUI::PrintHeader("ИТОГ ДНЯ 4");
    std::cout << "\nТы возвращаешься домой готовиться к следующему экзамену.\n";
    stats.intellect = std::min(stats.intellect + 5, 100);
    std::cout << "Интеллект +5\n";
    
    std::cout << "Четвертый день заканчивается. Ты ложишься спать.\n";
    
    // Сон восстанавливает силы
    stats.energy = GameConstants::MAX_STAT;
    stats.fatigue = 0;
    stats.hunger = std::min(stats.hunger + 10, 100);
    if (examResult == 2) {
        stats.stress = std::min(stats.stress + 10, 100);
    } else {
        stats.stress = std::max(stats.stress - 5, 0);
    }
    stats.clampAll();
    
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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        ConsoleUI::RenderScreen("АЛЛА", "Алла улыбается: «Спасибо, мне правда стало легче.\nТы очень поддерживаешь меня.»",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Алла", 10);
        state.getPlayer().getStats().romance += 5;
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
        state.getPlayer().getStats().romance += 5;
        state.getPlayer().getStats().intellect += 3;
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
        state.getPlayer().getStats().humanity -= 10;
        ConsoleUI::WaitForEnter();

        // Возможность купить ответы
        ConsoleUI::RenderScreen("ПОКУПКА ОТВЕТОВ",
            "Ты находишь Семёна. Он продаёт ответы за 300 руб.\n"
            "Купишь?",
            {"Купить ответы за 300 руб",
             "Передумать и надеяться на свои силы"},
            state.getPlayer());

        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 1 && state.getPlayer().getStats().money >= 300) {
            state.getPlayer().getStats().money -= 300;
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

    if (state.getPlayer().hasFlag("bought_answers") && score < 50) {
        ConsoleUI::RenderScreen("ОТВЕТЫ", "Купленные ответы помогли! +15 баллов.", {}, state.getPlayer());
        score = std::min(100, score + 15);
        state.getPlayer().setGrade(4, score);
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

    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice == 1) {
        ConsoleUI::RenderScreen("БАР", "Вы идёте в бар, берёте по пиву.\nОбсуждаете экзамены, смеётесь над преподавателями.\nНастроение улучшается!",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 8);
        state.getPlayer().getStats().stress -= 15;
        state.getPlayer().getStats().fatigue += 10;
        state.getPlayer().getStats().money -= 200;
    } else if (choice == 3) {
        ConsoleUI::RenderScreen("ПРОГУЛКА", "Вы гуляете полчаса, обсуждаете планы.\nБулат понимает, что тебе нужно готовиться.",
            {}, state.getPlayer());
        state.getPlayer().modifyRelation("Булат", 5);
        state.getPlayer().getStats().stress -= 5;
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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        ConsoleUI::RenderScreen("ДОМА",
            "Ты отдыхаешь дома: спишь до обеда, смотришь\n"
            "Netflix, заказываешь пиццу. Благодать.",
            {}, state.getPlayer());
        state.getPlayer().getStats().energy += 40;
        state.getPlayer().getStats().fatigue -= 30;
        state.getPlayer().getStats().hunger += 10;
        state.getPlayer().getStats().stress -= 15;
        state.getPlayer().advanceTime(360);
        break;
    case 2: {
        ConsoleUI::RenderScreen("РАБОТА",
            "Ты идёшь на подработку в кофейню.\n"
            "Несколько часов за стойкой — и деньги в кармане.",
            {"Работать бариста (+300 руб)",
             "Работать курьером (+400 руб, но устанешь)"},
            state.getPlayer());

        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 1) {
            state.getPlayer().getStats().money += 300;
            state.getPlayer().getStats().fatigue += 10;
            state.getPlayer().getStats().energy -= 10;
        } else {
            state.getPlayer().getStats().money += 400;
            state.getPlayer().getStats().fatigue += 20;
            state.getPlayer().getStats().energy -= 20;
        }
        state.getPlayer().advanceTime(240);
        break;
    }
    case 3:
        ConsoleUI::RenderScreen("ПРОГУЛКА",
            "Ты гуляешь по городу. Солнце, птицы, мороженое.\n"
            "Жизнь прекрасна, когда нет экзаменов.",
            {}, state.getPlayer());
        state.getPlayer().getStats().stress -= 15;
        state.getPlayer().getStats().energy += 5;
        state.getPlayer().advanceTime(120);
        break;
    case 4: {
        ConsoleUI::RenderScreen("ДРУЗЬЯ",
            "Ты встречаешься с Булатом и Семёном.\n"
            "Булат предлагает сыграть в настолки.",
            {"Согласиться — отдохнуть и повеселиться",
             "Обсуждать учёбу и стратегию"},
            state.getPlayer());

        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 1) {
            state.getPlayer().getStats().stress -= 20;
            state.getPlayer().modifyRelation("Булат", 5);
            state.getPlayer().modifyRelation("Семён", 3);
        } else {
            state.getPlayer().getStats().intellect += 4;
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

        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 1) {
            state.getPlayer().modifyRelation("Алла", 5);
            state.getPlayer().getStats().romance += 3;
        } else if (choice == 2) {
            state.getPlayer().modifyRelation("Алла", 8);
            state.getPlayer().getStats().romance += 5;
            state.getPlayer().setFlag("invited_alla_to_walk", true);
        } else if (choice == 3) {
            state.getPlayer().modifyRelation("Алла", 3);
            state.getPlayer().getStats().intellect += 2;
        } else {
            state.getPlayer().modifyRelation("Алла", 10);
            state.getPlayer().getStats().romance += 8;
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

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 2) {
            ConsoleUI::RenderScreen("ОТКАЗ",
                "Алла разочарованно вздыхает:\n"
                "«Ну... ладно. Понимаю. В другой раз.»\n"
                "Ты чувствуешь, что упустил что-то важное.",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", -10);
            state.getPlayer().getStats().romance -= 5;
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

        std::cin >> choice;
        std::cin.ignore(10000, '\n');

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

        if (state.getPlayer().getStats().money >= moneyCost) {
            state.getPlayer().getStats().money -= moneyCost;
            ConsoleUI::RenderScreen("СВИДАНИЕ", dateResult, {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 15);
            state.getPlayer().getStats().romance += 12;
            state.getPlayer().getStats().stress -= 15;
        } else {
            ConsoleUI::RenderScreen("СВИДАНИЕ",
                "У тебя не хватает денег на запланированное.\n"
                "Приходится идти просто гулять.\n"
                "Алла говорит, что ей всё равно нравится проводить с тобой время.",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 8);
            state.getPlayer().getStats().romance += 5;
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
            state.getPlayer().getStats().romance += 10;
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

        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1:
            ConsoleUI::RenderScreen("ДОЖДЬ",
                "Вы идёте под одним зонтом, прижавшись друг к другу.\n"
                "Алла смеётся: «Уютно, хоть и мокро.»",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 8);
            state.getPlayer().getStats().romance += 5;
            break;
        case 2:
            ConsoleUI::RenderScreen("ДОЖДЬ",
                "Алла укрывается зонтом, а ты мокнешь под дождём.\n"
                "Она смотрит на тебя с нежностью: «Ты такой заботливый...»",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 12);
            state.getPlayer().getStats().romance += 8;
            state.getPlayer().getStats().health -= 5;
            break;
        case 3:
            ConsoleUI::RenderScreen("ДОЖДЬ",
                "Алла смеётся: «С тобой даже дождь — приключение!»\n"
                "Вы оба промокаете, но настроение отличное.",
                {}, state.getPlayer());
            state.getPlayer().modifyRelation("Алла", 10);
            state.getPlayer().getStats().romance += 7;
            state.getPlayer().getStats().stress -= 10;
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

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1:
            ConsoleUI::RenderScreen("ПОДГОТОВКА", "Ты зубришь комп. сети.", {}, state.getPlayer());
            state.getPlayer().getStats().intellect += 8;
            state.getPlayer().getStats().fatigue += 10;
            break;
        case 2:
            interactWithBulat();
            break;
        case 3:
            ConsoleUI::RenderScreen("ОТДЫХ", "Ты отдыхаешь весь день.", {}, state.getPlayer());
            state.getPlayer().getStats().energy += 20;
            state.getPlayer().getStats().stress -= 10;
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
    std::cout << BOX_V "  Ваш выбор [0-4]: ";
    std::cout << std::string(40, ' ') << BOX_V "\n";
    std::cout << BOX_BL << std::string(78, BOX_H[0]) << BOX_BR "\n";

    // Курсор уже в нужном месте после "Ваш выбор"
    std::cout << "\r                                                                                \r";
    std::cout << "  Ваш выбор: ";
    int finalChoice;
    std::cin >> finalChoice;
    std::cin.ignore(10000, '\n');

    if (finalChoice == 4 && state.getPlayer().getDebts() >= 3) {
        ConsoleUI::RenderScreen("ПУТЬ",
            "Возможно, тебе стоит взять паузу и подумать...\n"
            "Армия — тоже вариант.",
            {}, state.getPlayer());
        state.getPlayer().setFlag("army_path", true);
        ConsoleUI::WaitForEnter();
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 8);

    // Переход к концовке
    state.getPlayer().nextDay();
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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    auto& s = state.getPlayer().getStats();

    switch (choice) {
    case 1:
        s.energy += GameConstants::SLEEP_ENERGY_GAIN;
        s.fatigue -= GameConstants::SLEEP_FATIGUE_REDUCE;
        s.stress -= GameConstants::SLEEP_STRESS_REDUCE;

        // Сон тратит сытость.
        s.hunger -= GameConstants::SLEEP_HUNGER_LOSS;

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
        } else if (s.money < GameConstants::EAT_MONEY_COST) {
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
        s.intellect += GameConstants::STUDY_INTELLECT_GAIN;
        s.fatigue += GameConstants::STUDY_FATIGUE_COST;
        s.energy -= GameConstants::STUDY_ENERGY_COST;

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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1: interactWithAlla(); break;
    case 2: interactWithBulat(); break;
    case 3: interactWithSemen(); break;
    case 4: interactWithArtem(); break;
    case 5:
        ConsoleUI::RenderScreen("БИБЛИОТЕКА", "Ты занимаешься в библиотеке.", {}, state.getPlayer());
        state.getPlayer().getStats().intellect += 5;
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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1: state.getPlayer().setLocation(LocationID::University); break;
    case 2: state.getPlayer().setLocation(LocationID::Canteen); break;
    case 3: state.getPlayer().setLocation(LocationID::Shop); break;
    case 4: state.getPlayer().setLocation(LocationID::FlowerShop); break;
    case 5:
        ConsoleUI::RenderScreen("ПРОГУЛКА", "Ты гуляешь. Встречаешь знакомых.", {}, state.getPlayer());
        state.getPlayer().getStats().stress -= 5;
        state.getPlayer().advanceTime(30);
        ConsoleUI::WaitForEnter();
        break;
    }
}

void Game::handleCanteenLocation() {
    ConsoleUI::PrintHeader("СТОЛОВАЯ");
    ConsoleUI::ShowLocationArt(LocationID::Canteen);

    ConsoleUI::RenderScreen(
        "СТОЛОВАЯ",
        "Что будешь заказывать?",
        {
            "Комплексный обед (150 руб, +50 сытости)",
            "Чай с булочкой (50 руб, +20 сытости)",
            "Ничего, просто посидеть"
        },
        state.getPlayer(),
        "",
        "",
        "Столовая"
    );

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    auto& s = state.getPlayer().getStats();

    switch (choice) {
    case 1:
        if (s.money >= 150) {
            s.money -= 150;
            s.hunger = std::min(GameConstants::MAX_HUNGER, s.hunger + 50);
            s.energy += 15;

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
        if (s.money >= 50) {
            s.money -= 50;
            s.hunger = std::min(GameConstants::MAX_HUNGER, s.hunger + 20);
            s.energy += 5;

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
        s.stress -= 3;
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

    ConsoleUI::RenderScreen(
        "МАГАЗИН",
        "Что купить?",
        {
            "Продукты (200 руб, +30 сытости)",
            "Канцтовары (100 руб, +2 интеллект)",
            "Книгу по C++ (300 руб, +8 интеллект)",
            "Выйти"
        },
        state.getPlayer(),
        "",
        "",
        "Магазин"
    );

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    auto& s = state.getPlayer().getStats();

    switch (choice) {
    case 1:
        if (s.money >= 200) {
            s.money -= 200;
            s.hunger = std::min(GameConstants::MAX_HUNGER, s.hunger + 30);

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
        if (s.money >= 100) {
            s.money -= 100;
            s.intellect += 2;

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
        if (s.money >= 300) {
            s.money -= 300;
            s.intellect += 8;

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

    if (state.getPlayer().getStats().money >= GameConstants::FLOWER_COST) {
        ConsoleUI::RenderScreen("ЦВЕТЫ",
            "В цветочном магазине прекрасный аромат.\n"
            "Продавщица: «Что желаете?»\n"
            "Букет роз стоит 300 руб.",
            {"Купить букет цветов (300 руб)", "Выйти"},
            state.getPlayer());

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 1) {
            state.getPlayer().getStats().money -= GameConstants::FLOWER_COST;
            state.getPlayer().setFlag("has_flowers", true);
            ConsoleUI::RenderScreen("ЦВЕТЫ",
                "Ты купил прекрасный букет!\n"
                "Кому подарить?",
                {"Подарить Алле", "Оставить себе"},
                state.getPlayer());

            int subChoice;
            std::cin >> subChoice;
            std::cin.ignore(10000, '\n');

            if (subChoice == 1) {
                state.getPlayer().modifyRelation("Алла", 15);
                state.getPlayer().getStats().romance += 10;
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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

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

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice > 0 && choice <= static_cast<int>(choices.size())) {
        const auto& selected = choices[choice - 1];
        RelationshipSystem::ApplyChoiceEffect(state.getPlayer(), "Артём", selected.effects);
        std::cout << "\n" << selected.resultingText << "\n";
    }

    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}
