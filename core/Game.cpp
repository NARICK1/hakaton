#include "Game.h"
#include "../ui/ConsoleUI.h"
#include "../ui/Menu.h"
#include "../systems/EndingSystem.h"
#include "../systems/RelationshipSystem.h"
#include "../systems/FatigueSystem.h"
#include "../systems/HungerSystem.h"
#include "../exams/Exam.h"
#include <iostream>

Game::Game() {
    ConsoleUI::SetConsoleUTF8();
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
                break;
            case 2:
                loadGame();
                if (state.isGameStarted()) {
                    initNPCs();
                    state.setPhase(GamePhase::Playing);
                }
                break;
            case 3:
                Menu::ShowControls();
                break;
            case 0:
                running = false;
                break;
            }
            break;

        case GamePhase::Playing: {
            int day = state.getPlayer().getCurrentDay();

            switch (day) {
            case 1: runDay1(); break;
            case 2: runDay2(); break;
            case 3: runDay3(); break;
            case 4: runDay4(); break;
            case 5: runDay5(); break;
            case 6: runDay6(); break;
            case 7: runDay7(); break;
            case 8: runDay8(); break;
            default:
                // После 8 дня — оценка концовки
                auto ending = EndingSystem::EvaluateEnding(state.getPlayer());
                state.setGameOverReason(ending);
                state.setPhase(GamePhase::GameOver);
                break;
            }

            // Проверяем концовку
            checkGameOver();

            // Если день закончился и игра не окончена, переходим к следующему дню
            if (state.getPhase() == GamePhase::Playing) {
                state.getPlayer().nextDay();
                applyDailySystems();
                saveGame();
            }
            break;
        }

        case GamePhase::GameOver: {
            Menu::ShowGameOverMenu(
                EndingSystem::GetEndingText(state.getGameOverReason()));
            int choice = Menu::MainMenuChoice();
            if (choice == 1) {
                initGame();
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
    state.getPlayer().applyBuffs();
}

void Game::checkGameOver() {
    auto reason = EndingSystem::CheckEnding(state.getPlayer());
    if (reason != GameOverCondition::None) {
        state.setGameOverReason(reason);
        state.setPhase(GamePhase::GameOver);
    }
}

void Game::saveGame() {
    SaveManager::SaveGame(state);
}

void Game::loadGame() {
    SaveManager::LoadGame(state);
    if (state.isGameStarted()) {
        state.setPhase(GamePhase::Playing);
    }
}

// ==================== СЦЕНЫ ПО ДНЯМ ====================

void Game::runDay1() {
    ConsoleUI::PrintDayHeader(1, "История — первый экзамен");
    std::cout << "\nПонедельник. Первый экзамен. История.\n";
    std::cout << "Ты просыпаешься и понимаешь: сегодня решается твоя судьба.\n";
    ConsoleUI::WaitForEnter();

    // Утро
    ConsoleUI::PrintHeader("УТРО");
    std::cout << "Булат стучит в дверь: «" << state.getPlayer().getName()
              << ", вставай! Опоздаем на экзамен!»\n\n";
    std::cout << "Твои действия:\n";
    std::cout << "1. Вскочить и быстро собраться\n";
    std::cout << "2. Попросить Булата подождать 5 минут\n";
    std::cout << "3. Сказать, что идёшь один\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        std::cout << "\nТы быстро собираешься. Булат доволен твоей скоростью.\n";
        state.getPlayer().modifyRelation("Булат", 3);
        state.getPlayer().getStats().energy -= 5;
        break;
    case 2:
        std::cout << "\nБулат ждёт. Вы идёте вместе, обсуждая историю.\n";
        state.getPlayer().modifyRelation("Булат", 5);
        state.getPlayer().getStats().intellect += 2;
        break;
    case 3:
        std::cout << "\nБулат уходит один. Ты чувствуешь лёгкую неловкость.\n";
        state.getPlayer().modifyRelation("Булат", -3);
        break;
    }

    ConsoleUI::WaitForEnter();

    // Экзамен по истории
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО ИСТОРИИ");
    HistoryExam historyExam;
    historyExam.runExam(state.getPlayer());

    ConsoleUI::WaitForEnter();

    // После экзамена
    ConsoleUI::PrintHeader("ПОСЛЕ ЭКЗАМЕНА");
    std::cout << "Экзамен позади. Ты встречаешь Аллу в коридоре.\n";
    alla->getDialog(state.getPlayer());
    interactWithAlla();

    ConsoleUI::WaitForEnter();

    // Случайное событие
    eventManager.tryTriggerEvent(state.getPlayer(), 1);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 1");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

void Game::runDay2() {
    ConsoleUI::PrintDayHeader(2, "Язык и математика программирования");
    std::cout << "\nВторой экзамен — ЯиМП. Нужно собраться.\n";
    ConsoleUI::WaitForEnter();

    // Выбор локации
    bool dayEnded = false;
    while (!dayEnded) {
        ConsoleUI::ClearScreen();
        ConsoleUI::PrintDayHeader(2, "Язык и математика программирования");
        ConsoleUI::PrintPlayerStats(state.getPlayer());

        std::cout << "Куда хочешь пойти?\n";
        std::cout << "1. Университет (экзамен)\n";
        std::cout << "2. Дом (подготовиться)\n";
        std::cout << "3. Столовая (поесть)\n";
        std::cout << "4. Магазин\n";
        std::cout << "5. Улица (проветриться)\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1: {
            ConsoleUI::PrintHeader("ЭКЗАМЕН ПО ЯИМП");
            YAMPExam yampExam;
            yampExam.runExam(state.getPlayer());
            eventManager.tryTriggerEvent(state.getPlayer(), 2);
            ConsoleUI::WaitForEnter();
            dayEnded = true;
            break;
        }
        case 2: {
            ConsoleUI::PrintHeader("ПОДГОТОВКА ДОМА");
            std::cout << "Ты решаешь задачи и повторяешь материал.\n";
            state.getPlayer().getStats().intellect += 5;
            state.getPlayer().getStats().energy -= 10;
            state.getPlayer().advanceTime(120);
            ConsoleUI::WaitForEnter();
            break;
        }
        case 3: {
            handleCanteenLocation();
            break;
        }
        case 4: {
            handleShopLocation();
            break;
        }
        case 5: {
            std::cout << "\nТы гуляешь по улице, размышляя о жизни.\n";
            state.getPlayer().getStats().stress -= 5;
            state.getPlayer().advanceTime(30);
            ConsoleUI::WaitForEnter();
            break;
        }
        }

        checkGameOver();
        if (state.getPhase() != GamePhase::Playing) return;
    }

    // Случайное событие
    eventManager.tryTriggerEvent(state.getPlayer(), 2);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 2");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

void Game::runDay3() {
    ConsoleUI::PrintDayHeader(3, "Подготовка к дискретной математике");
    std::cout << "\nДень подготовки к дискретной математике.\n";
    std::cout << "Сегодня нет экзаменов. Можно подготовиться.\n";
    ConsoleUI::WaitForEnter();

    bool dayEnded = false;
    while (!dayEnded) {
        ConsoleUI::ClearScreen();
        ConsoleUI::PrintDayHeader(3, "Подготовка к дискретной математике");
        ConsoleUI::PrintPlayerStats(state.getPlayer());

        std::cout << "Чем займёшься?\n";
        std::cout << "1. Изучать дискретную математику (библиотека)\n";
        std::cout << "2. Встретиться с друзьями\n";
        std::cout << "3. Пойти в столовую\n";
        std::cout << "4. Пойти в магазин\n";
        std::cout << "5. Гулять по улице\n";
        std::cout << "6. Лечь спать (закончить день)\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1: {
            std::cout << "\nТы идёшь в библиотеку и погружаешься в изучение графов.\n";
            state.getPlayer().getStats().intellect += 8;
            state.getPlayer().getStats().fatigue += 15;
            state.getPlayer().advanceTime(180);
            ConsoleUI::WaitForEnter();
            break;
        }
        case 2: {
            ConsoleUI::PrintHeader("ВСТРЕЧА С ДРУЗЬЯМИ");
            std::cout << "Ты встречаешь Булата и Семёна.\n";

            std::cout << "1. Обсуждать учёбу\n";
            std::cout << "2. Просто болтать о жизни\n";
            std::cout << "3. Попросить помощи с дискреткой\n";

            int subChoice;
            std::cin >> subChoice;
            std::cin.ignore(10000, '\n');

            if (subChoice == 1) {
                state.getPlayer().getStats().intellect += 3;
                state.getPlayer().modifyRelation("Булат", 3);
                state.getPlayer().modifyRelation("Семён", 3);
            } else if (subChoice == 2) {
                state.getPlayer().getStats().stress -= 10;
                state.getPlayer().modifyRelation("Булат", 5);
                state.getPlayer().modifyRelation("Семён", 3);
            } else if (subChoice == 3) {
                state.getPlayer().getStats().intellect += 6;
                state.getPlayer().modifyRelation("Булат", 5);
                state.getPlayer().modifyRelation("Семён", 5);
            }

            state.getPlayer().advanceTime(90);
            ConsoleUI::WaitForEnter();
            break;
        }
        case 3:
            handleCanteenLocation();
            break;
        case 4:
            handleShopLocation();
            break;
        case 5: {
            std::cout << "\nТы гуляешь по городу. Встречаешь Артёма.\n";
            interactWithArtem();
            state.getPlayer().getStats().stress -= 5;
            state.getPlayer().advanceTime(60);
            ConsoleUI::WaitForEnter();
            break;
        }
        case 6:
            dayEnded = true;
            break;
        }

        checkGameOver();
        if (state.getPhase() != GamePhase::Playing) return;
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 3);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 3");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

void Game::runDay4() {
    ConsoleUI::PrintDayHeader(4, "Дискретная математика");
    std::cout << "\nЭкзамен по дискретной математике.\n";
    std::cout << "Это один из самых сложных экзаменов.\n";
    ConsoleUI::WaitForEnter();

    // Встреча с Семёном перед экзаменом
    ConsoleUI::PrintHeader("ПЕРЕД ЭКЗАМЕНОМ");
    std::cout << "Семён подходит к тебе перед аудиторией:\n";
    std::cout << "«" << state.getPlayer().getName() << ", давай шпаргалку? "
              << "У меня есть отличная.\n";
    std::cout << "1. Взять шпаргалку\n";
    std::cout << "2. Отказаться — надеяться на свои знания\n";
    std::cout << "3. Попросить объяснить сложные моменты\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        std::cout << "\nТы берёшь шпаргалку. Семён одобрительно кивает.\n";
        state.getPlayer().modifyRelation("Семён", 5);
        state.getPlayer().setFlag("took_cheat_sheet");
        state.getPlayer().getStats().humanity -= 5;
        break;
    case 2:
        std::cout << "\nТы отказываешься. Семён пожимает плечами.\n";
        state.getPlayer().modifyRelation("Семён", -2);
        state.getPlayer().getStats().humanity += 5;
        break;
    case 3:
        std::cout << "\nСемён быстро объясняет сложные моменты.\n";
        state.getPlayer().getStats().intellect += 5;
        state.getPlayer().modifyRelation("Семён", 3);
        break;
    }

    ConsoleUI::WaitForEnter();

    // Экзамен
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО ДИСКРЕТНОЙ МАТЕМАТИКЕ");
    DiscreteExam discreteExam;
    int score = discreteExam.runExam(state.getPlayer());

    // Если взял шпаргалку, бонус
    if (state.getPlayer().hasFlag("took_cheat_sheet") && score < 50) {
        std::cout << "\nШпаргалка помогла! +10 баллов.\n";
        score = std::min(100, score + 10);
        state.getPlayer().setGrade(3, score);
    }

    ConsoleUI::WaitForEnter();

    // После экзамена
    ConsoleUI::PrintHeader("ПОСЛЕ ЭКЗАМЕНА");
    std::cout << "После экзамена ты встречаешь Аллу.\n";
    interactWithAlla();

    eventManager.tryTriggerEvent(state.getPlayer(), 4);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 4");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

void Game::runDay5() {
    ConsoleUI::PrintDayHeader(5, "Математический анализ");
    std::cout << "\nЭкзамен по матанализу.\n";
    std::cout << "Самый страшный экзамен для всех студентов.\n";
    ConsoleUI::WaitForEnter();

    // Утро перед экзаменом
    ConsoleUI::PrintHeader("УТРО ПЕРЕД ЭКЗАМЕНОМ");
    std::cout << "Алла подходит к тебе с обеспокоенным видом:\n";
    std::cout << "«Привет! Ты готов? Я так переживаю...\n";
    std::cout << "1. Подбодрить Аллу\n";
    std::cout << "2. Сказать, что сам в панике\n";
    std::cout << "3. Предложить готовиться вместе\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        std::cout << "\nАлла улыбается: «Спасибо, мне стало легче!»\n";
        state.getPlayer().modifyRelation("Алла", 8);
        state.getPlayer().getStats().romance += 3;
        break;
    case 2:
        std::cout << "\nАлла вздыхает: «Да уж, мы оба в одной лодке...»\n";
        state.getPlayer().modifyRelation("Алла", 2);
        break;
    case 3:
        std::cout << "\nВы вместе повторяете интегралы. Алла благодарна.\n";
        state.getPlayer().modifyRelation("Алла", 10);
        state.getPlayer().getStats().romance += 5;
        state.getPlayer().getStats().intellect += 3;
        break;
    }

    ConsoleUI::WaitForEnter();

    // Экзамен
    ConsoleUI::PrintHeader("ЭКЗАМЕН ПО МАТАНАЛИЗУ");
    CalculusExam calculusExam;
    calculusExam.runExam(state.getPlayer());

    ConsoleUI::WaitForEnter();

    // После экзамена встреча с Булатом
    ConsoleUI::PrintHeader("ПОСЛЕ ЭКЗАМЕНА");
    std::cout << "Булат зовёт тебя отметить окончание сложного экзамена.\n";
    std::cout << "1. Пойти с Булатом\n";
    std::cout << "2. Отказаться — нужно готовиться к следующему\n";
    std::cout << "3. Пойти, но ненадолго\n";

    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        std::cout << "\nВы хорошо проводите время. Настроение улучшается!\n";
        state.getPlayer().modifyRelation("Булат", 8);
        state.getPlayer().getStats().stress -= 15;
        state.getPlayer().getStats().fatigue += 10;
        break;
    case 2:
        std::cout << "\nБулат понимающе кивает: «Удачи с подготовкой!»\n";
        state.getPlayer().modifyRelation("Булат", -2);
        break;
    case 3:
        std::cout << "\nВы немного гуляете, но ты быстро возвращаешься домой.\n";
        state.getPlayer().modifyRelation("Булат", 5);
        state.getPlayer().getStats().stress -= 5;
        break;
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 5);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 5");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

void Game::runDay6() {
    ConsoleUI::PrintDayHeader(6, "Выходной");
    std::cout << "\nНаконец-то выходной! Можно отдохнуть.\n";
    std::cout << "Сегодня нет экзаменов. Ты сам решаешь, чем заняться.\n";
    ConsoleUI::WaitForEnter();

    state.getPlayer().setFlag("day7_available", true);

    bool dayEnded = false;
    while (!dayEnded) {
        ConsoleUI::ClearScreen();
        ConsoleUI::PrintDayHeader(6, "Выходной");
        ConsoleUI::PrintPlayerStats(state.getPlayer());

        std::cout << "Чем хочешь заняться?\n";
        std::cout << "1. Отдыхать дома (спать, есть, смотреть сериалы)\n";
        std::cout << "2. Пойти гулять по городу\n";
        std::cout << "3. Встретиться с друзьями\n";
        std::cout << "4. Сходить в магазин\n";
        std::cout << "5. Цветочный магазин\n";
        std::cout << "6. Закончить день\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1: {
            std::cout << "\nТы отдыхаешь дома. Силы восстанавливаются.\n";
            state.getPlayer().getStats().energy += 40;
            state.getPlayer().getStats().fatigue -= 30;
            state.getPlayer().getStats().hunger += 10;
            state.getPlayer().getStats().stress -= 15;
            state.getPlayer().advanceTime(240);
            ConsoleUI::WaitForEnter();
            break;
        }
        case 2: {
            std::cout << "\nТы гуляешь по городу. Встречаешь Аллу!\n";
            interactWithAlla();

            std::cout << "\nПрогулка продолжается...\n";
            state.getPlayer().getStats().stress -= 10;
            state.getPlayer().advanceTime(90);
            ConsoleUI::WaitForEnter();
            break;
        }
        case 3: {
            std::cout << "\nТы встречаешься с Булатом и Семёном.\n";
            std::cout << "1. Играть в настольные игры\n";
            std::cout << "2. Обсуждать учёбу\n";
            std::cout << "3. Просто болтать\n";

            int subChoice;
            std::cin >> subChoice;
            std::cin.ignore(10000, '\n');

            if (subChoice == 1) {
                state.getPlayer().getStats().stress -= 15;
                state.getPlayer().modifyRelation("Булат", 5);
                state.getPlayer().modifyRelation("Семён", 3);
            } else if (subChoice == 2) {
                state.getPlayer().getStats().intellect += 4;
                state.getPlayer().modifyRelation("Семён", 5);
            } else {
                state.getPlayer().getStats().stress -= 8;
                state.getPlayer().modifyRelation("Булат", 3);
                state.getPlayer().modifyRelation("Семён", 3);
            }
            state.getPlayer().advanceTime(120);
            ConsoleUI::WaitForEnter();
            break;
        }
        case 4:
            handleShopLocation();
            break;
        case 5:
            handleFlowerShopLocation();
            break;
        case 6:
            dayEnded = true;
            break;
        }

        checkGameOver();
        if (state.getPhase() != GamePhase::Playing) return;
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 6);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 6 — ВЫХОДНОГО");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

void Game::runDay7() {
    ConsoleUI::PrintDayHeader(7, "Свидание");
    std::cout << "\nСегодня особенный день.\n";

    if (state.getPlayer().getRelation("Алла") >= 60 && state.getPlayer().getStats().romance >= 30) {
        std::cout << "Алла сама подходит к тебе и говорит:\n";
        std::cout << "«Сегодня я хочу провести этот день с тобой.\n";
        std::cout << "Если ты, конечно, не против...»\n";

        ConsoleUI::WaitForEnter();

        ConsoleUI::PrintHeader("СВИДАНИЕ С АЛЛОЙ");
        std::cout << "Куда пойдёте?\n";
        std::cout << "1. Кафе — уютная атмосфера\n";
        std::cout << "2. Парк — прогулка на свежем воздухе\n";
        std::cout << "3. Кино — классика свиданий\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1:
            std::cout << "\nВы сидите в уютном кафе, разговариваете обо всём.\n";
            std::cout << "Алла рассказывает о своих мечтах.\n";
            state.getPlayer().modifyRelation("Алла", 15);
            state.getPlayer().getStats().romance += 15;
            state.getPlayer().getStats().money -= 300;
            break;
        case 2:
            std::cout << "\nВы гуляете по парку, кормите уток.\n";
            std::cout << "Алла смеётся над твоими шутками.\n";
            state.getPlayer().modifyRelation("Алла", 12);
            state.getPlayer().getStats().romance += 12;
            state.getPlayer().getStats().stress -= 10;
            break;
        case 3:
            std::cout << "\nВы смотрите фильм. Алла прижимается к тебе.\n";
            std::cout << "Это был отличный вечер.\n";
            state.getPlayer().modifyRelation("Алла", 10);
            state.getPlayer().getStats().romance += 10;
            state.getPlayer().getStats().money -= 200;
            break;
        }

        ConsoleUI::WaitForEnter();

        // После свидания
        ConsoleUI::PrintHeader("ПОСЛЕ СВИДАНИЯ");
        std::cout << "Алла берёт тебя за руку:\n";
        std::cout << "«Спасибо за этот день. Ты особенный человек.»\n";
        std::cout << "Ты чувствуешь, что между вами что-то большее...\n";

        if (state.getPlayer().getStats().romance >= 65 &&
            state.getPlayer().getRelation("Алла") >= 75) {
            state.getPlayer().setFlag("romantic_ending", true);
            std::cout << "\nВы целуетесь. Это магия.\n";
        }

        ConsoleUI::WaitForEnter();
        state.getPlayer().setFlag("day7_done", true);
    } else {
        std::cout << "Увы, у тебя недостаточно высокие отношения с Аллой.\n";
        std::cout << "Она не пришла на встречу. Возможно, в следующий раз...\n";
        ConsoleUI::WaitForEnter();
    }

    // Если Алла не пришла, можно заняться другими делами
    if (!state.getPlayer().hasFlag("day7_done")) {
        ConsoleUI::PrintHeader("ДЕНЬ БЕЗ СВИДАНИЯ");
        std::cout << "Чем займёшься?\n";
        std::cout << "1. Готовиться к последним экзаменам\n";
        std::cout << "2. Пойти гулять с Булатом\n";
        std::cout << "3. Просто отдыхать\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1:
            state.getPlayer().getStats().intellect += 8;
            state.getPlayer().getStats().fatigue += 10;
            break;
        case 2:
            interactWithBulat();
            break;
        case 3:
            state.getPlayer().getStats().energy += 20;
            state.getPlayer().getStats().stress -= 10;
            break;
        }
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 7);

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 7");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

void Game::runDay8() {
    ConsoleUI::PrintDayHeader(8, "Последний экзамен и подведение итогов");
    std::cout << "\nПоследний день сессии. Остался экзамен по комп. сетям.\n";
    std::cout << "Сегодня решается всё.\n";
    ConsoleUI::WaitForEnter();

    // Последний экзамен
    ConsoleUI::PrintHeader("ПОСЛЕДНИЙ ЭКЗАМЕН");
    NetworksExam networksExam;
    networksExam.runExam(state.getPlayer());

    ConsoleUI::WaitForEnter();

    // Подведение итогов
    ConsoleUI::PrintHeader("ПОДВЕДЕНИЕ ИТОГОВ");

    const auto& stats = state.getPlayer().getStats();
    std::cout << "Сессия позади. Давай посмотрим, как всё прошло.\n\n";

    int totalScore = 0;
    for (int i = 1; i <= 5; i++) {
        int g = state.getPlayer().getGrade(i);
        if (g > 0) {
            std::cout << "Экзамен " << i << ": " << g << " баллов\n";
            totalScore += g;
        } else {
            std::cout << "Экзамен " << i << ": не сдан\n";
        }
    }

    std::cout << "\nИтоговые характеристики:\n";
    std::cout << "Интеллект: " << stats.intellect << "\n";
    std::cout << "Человечность: " << stats.humanity << "\n";
    std::cout << "Романтика: " << stats.romance << "\n";
    std::cout << "Отношения с Аллой: " << state.getPlayer().getRelation("Алла") << "\n";
    std::cout << "Долгов: " << state.getPlayer().getDebts() << "\n";

    ConsoleUI::WaitForEnter();

    // Финальный выбор
    ConsoleUI::PrintHeader("ФИНАЛЬНЫЙ ВЫБОР");
    std::cout << "Что ты чувствуешь после сессии?\n";
    std::cout << "1. Гордость — ты справился со всем!\n";
    std::cout << "2. Облегчение — наконец-то всё кончилось.\n";
    std::cout << "3. Усталость — ты выжат как лимон.\n";
    std::cout << "4. Пустоту — что дальше?\n";

    int finalChoice;
    std::cin >> finalChoice;
    std::cin.ignore(10000, '\n');

    if (finalChoice == 4 && state.getPlayer().getDebts() >= 3) {
        std::cout << "\nВозможно, тебе стоит взять паузу и подумать...\n";
        state.getPlayer().setFlag("army_path", true);
    }

    eventManager.tryTriggerEvent(state.getPlayer(), 8);

    // Переход к концовке
    state.getPlayer().nextDay(); // день 9 — вызовет оценку концовки
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
    std::cout << "Ты дома. Можно отдохнуть.\n";
    std::cout << "1. Лечь спать\n";
    std::cout << "2. Поесть\n";
    std::cout << "3. Позаниматься\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        std::cout << "\nТы хорошо выспался.\n";
        state.getPlayer().getStats().energy += GameConstants::SLEEP_ENERGY_GAIN;
        state.getPlayer().getStats().fatigue -= GameConstants::SLEEP_FATIGUE_REDUCE;
        state.getPlayer().getStats().stress -= GameConstants::SLEEP_STRESS_REDUCE;
        state.getPlayer().getStats().hunger += GameConstants::SLEEP_HUNGER_INCREASE;
        state.getPlayer().advanceTime(480);
        break;
    case 2:
        HungerSystem::Eat(state.getPlayer(), GameConstants::EAT_MONEY_COST);
        std::cout << "Ты поел.\n";
        break;
    case 3:
        std::cout << "\nТы занимаешься.\n";
        state.getPlayer().getStats().intellect += GameConstants::STUDY_INTELLECT_GAIN;
        state.getPlayer().getStats().fatigue += GameConstants::STUDY_FATIGUE_COST;
        state.getPlayer().getStats().energy -= GameConstants::STUDY_ENERGY_COST;
        state.getPlayer().advanceTime(120);
        break;
    }
    state.getPlayer().getStats().clampAll();
}

void Game::handleUniversityLocation() {
    ConsoleUI::PrintHeader("УНИВЕРСИТЕТ");
    std::cout << "Ты в университете. Кого встретишь?\n";
    std::cout << "1. Аллу\n";
    std::cout << "2. Булата\n";
    std::cout << "3. Семёна\n";
    std::cout << "4. Артёма\n";
    std::cout << "5. Пойти в библиотеку\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1: interactWithAlla(); break;
    case 2: interactWithBulat(); break;
    case 3: interactWithSemen(); break;
    case 4: interactWithArtem(); break;
    case 5:
        std::cout << "\nТы занимаешься в библиотеке.\n";
        state.getPlayer().getStats().intellect += 5;
        state.getPlayer().advanceTime(120);
        break;
    }
    state.getPlayer().advanceTime(30);
}

void Game::handleStreetLocation() {
    ConsoleUI::PrintHeader("УЛИЦА");
    std::cout << "Ты на улице. Куда пойти?\n";
    std::cout << "1. В университет\n";
    std::cout << "2. В столовую\n";
    std::cout << "3. В магазин\n";
    std::cout << "4. В цветочный магазин\n";
    std::cout << "5. Просто гулять\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1: state.getPlayer().setLocation(LocationID::University); break;
    case 2: state.getPlayer().setLocation(LocationID::Canteen); break;
    case 3: state.getPlayer().setLocation(LocationID::Shop); break;
    case 4: state.getPlayer().setLocation(LocationID::FlowerShop); break;
    case 5:
        std::cout << "\nТы гуляешь. Встречаешь знакомых.\n";
        state.getPlayer().getStats().stress -= 5;
        state.getPlayer().advanceTime(30);
        break;
    }
}

void Game::handleCanteenLocation() {
    ConsoleUI::PrintHeader("СТОЛОВАЯ");
    std::cout << "Ты в столовой. Что будешь заказывать?\n";
    std::cout << "1. Комплексный обед (150 руб.)\n";
    std::cout << "2. Чай с булочкой (50 руб.)\n";
    std::cout << "3. Ничего, просто посидеть\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        if (state.getPlayer().getStats().money >= 150) {
            state.getPlayer().getStats().money -= 150;
            state.getPlayer().getStats().hunger = std::max(0, state.getPlayer().getStats().hunger - 50);
            state.getPlayer().getStats().energy += 15;
            std::cout << "\nВкусный обед!\n";
        } else {
            std::cout << "\nНе хватает денег.\n";
        }
        break;
    case 2:
        if (state.getPlayer().getStats().money >= 50) {
            state.getPlayer().getStats().money -= 50;
            state.getPlayer().getStats().hunger = std::max(0, state.getPlayer().getStats().hunger - 20);
            state.getPlayer().getStats().energy += 5;
            std::cout << "\nЧай с булочкой — отличный перекус.\n";
        } else {
            std::cout << "\nНе хватает денег.\n";
        }
        break;
    case 3:
        std::cout << "\nТы просто сидишь и отдыхаешь.\n";
        state.getPlayer().getStats().stress -= 3;
        break;
    }
    state.getPlayer().advanceTime(30);
    state.getPlayer().getStats().clampAll();
}

void Game::handleShopLocation() {
    ConsoleUI::PrintHeader("МАГАЗИН");
    std::cout << "Ты в магазине. Что купить?\n";
    std::cout << "1. Продукты (200 руб.)\n";
    std::cout << "2. Канцтовары (100 руб.)\n";
    std::cout << "3. Книгу по программированию (300 руб.)\n";
    std::cout << "0. Выйти\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    switch (choice) {
    case 1:
        if (state.getPlayer().getStats().money >= 200) {
            state.getPlayer().getStats().money -= 200;
            state.getPlayer().getStats().hunger = std::max(0, state.getPlayer().getStats().hunger - 30);
            std::cout << "\nКупил продукты.\n";
        } else {
            std::cout << "\nНе хватает денег.\n";
        }
        break;
    case 2:
        if (state.getPlayer().getStats().money >= 100) {
            state.getPlayer().getStats().money -= 100;
            state.getPlayer().getStats().intellect += 2;
            std::cout << "\nКупил канцтовары.\n";
        } else {
            std::cout << "\nНе хватает денег.\n";
        }
        break;
    case 3:
        if (state.getPlayer().getStats().money >= 300) {
            state.getPlayer().getStats().money -= 300;
            state.getPlayer().getStats().intellect += 8;
            std::cout << "\nКупил книгу по программированию.\n";
        } else {
            std::cout << "\nНе хватает денег.\n";
        }
        break;
    }
    state.getPlayer().advanceTime(20);
    state.getPlayer().getStats().clampAll();
}

void Game::handleFlowerShopLocation() {
    ConsoleUI::PrintHeader("ЦВЕТОЧНЫЙ МАГАЗИН");
    std::cout << "В цветочном магазине красивый аромат.\n";
    std::cout << "Продавщица: «Что желаете?»\n";

    if (state.getPlayer().getStats().money >= GameConstants::FLOWER_COST) {
        std::cout << "1. Купить букет цветов (300 руб.)\n";
        std::cout << "0. Выйти\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 1) {
            state.getPlayer().getStats().money -= GameConstants::FLOWER_COST;
            state.getPlayer().setFlag("has_flowers", true);
            std::cout << "\nТы купил прекрасный букет!\n";
            std::cout << "Интересно, кому его подарить?\n";

            std::cout << "1. Подарить Алле\n";
            std::cout << "2. Оставить себе\n";

            int subChoice;
            std::cin >> subChoice;
            std::cin.ignore(10000, '\n');

            if (subChoice == 1) {
                state.getPlayer().modifyRelation("Алла", 15);
                state.getPlayer().getStats().romance += 10;
                std::cout << "\nАлла в восторге от цветов!\n";
                state.getPlayer().setFlag("gave_flowers_to_alla", true);
            } else {
                std::cout << "\nЦветы будут стоять у тебя дома.\n";
            }
        }
    } else {
        std::cout << "У тебя недостаточно денег для покупки цветов.\n";
        ConsoleUI::WaitForEnter();
    }
    state.getPlayer().advanceTime(15);
}

// ==================== ВЗАИМОДЕЙСТВИЕ С NPC ====================

void Game::interactWithAlla() {
    ConsoleUI::PrintHeader("ОБЩЕНИЕ С АЛЛОЙ");
    std::cout << alla->getDialog(state.getPlayer()) << "\n\n";

    auto choices = alla->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << "0. Закончить разговор\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    if (choice > 0 && choice <= static_cast<int>(choices.size())) {
        const auto& selected = choices[choice - 1];
        RelationshipSystem::ApplyChoiceEffect(state.getPlayer(), "Алла", selected.effects);
        std::cout << "\n" << selected.resultingText << "\n";

        // Событие для романтической ветки
        if (selected.text.find("Пригласить на свидание") != std::string::npos) {
            ConsoleUI::WaitForEnter();
            ConsoleUI::PrintHeader("СВИДАНИЕ");
            std::cout << "Алла соглашается! Вы договариваетесь о встрече.\n";
            state.getPlayer().setFlag("day7_available", true);
        }
    }

    state.getPlayer().advanceTime(15);
    ConsoleUI::WaitForEnter();
}

void Game::interactWithBulat() {
    ConsoleUI::PrintHeader("ОБЩЕНИЕ С БУЛАТОМ");
    std::cout << bulat->getDialog(state.getPlayer()) << "\n\n";

    auto choices = bulat->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << "0. Закончить разговор\n";

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
    std::cout << semen->getDialog(state.getPlayer()) << "\n\n";

    auto choices = semen->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << "0. Закончить разговор\n";

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
    std::cout << artem->getDialog(state.getPlayer()) << "\n\n";

    auto choices = artem->getChoices(state.getPlayer());
    for (size_t i = 0; i < choices.size(); i++) {
        std::cout << (i + 1) << ". " << choices[i].text << "\n";
    }
    std::cout << "0. Закончить разговор\n";

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
