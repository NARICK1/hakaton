#include "Game.h"
#include "../ui/ConsoleUI.h"
#include "../ui/Menu.h"
#include "../systems/EndingSystem.h"
#include "../systems/RelationshipSystem.h"
#include "../systems/FatigueSystem.h"
#include "../systems/HungerSystem.h"
#include "../exams/Exam.h"
#include <iostream>
#include <random>
extern int historyGrade;
extern int yampGrade;
extern int discreteGrade;
extern int calculusGrade;
extern int failedExamsCount;
// ==================== ДЕНЬ 5 — МАТАНАЛИЗ ====================
void Game::runDay5() {
    ConsoleUI::PrintDayHeader(5, "Математический анализ");
    std::cout << "\nПятый день. Экзамен по математическому анализу.\n";
    std::cout << "Самый страшный экзамен для всех студентов.\n";
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();
    int fl = 0;
    int allaRelation = state.getPlayer().getRelation("Алла");

    // Показываем параметры
    ConsoleUI::PrintHeader("ПОДГОТОВКА К ЭКЗАМЕНУ");
    std::cout << "\nТвои параметры:\n";
    std::cout << "- Интеллект: " << stats.intellect << "/" << GameConstants::MAX_STAT << "\n";
    std::cout << "- Отношения с Аллой: " << allaRelation << "/100\n";
    std::cout << "- Деньги: " << stats.money << " руб.\n\n";
    ConsoleUI::WaitForEnter();

    // Выбор стратегии
    ConsoleUI::PrintHeader("ВЫБОР СТРАТЕГИИ");
    std::cout << "Как будешь сдавать экзамен?\n";
    std::cout << "1. Решать самостоятельно\n";
    std::cout << "2. Попросить помощь у Аллы\n";
    std::cout << "3. Списывать\n";
    std::cout << "4. Купить ответы\n";

    int choice;
    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    int examResult = 0;
    int examScore = 0;

    switch (choice) {
    case 1: { // САМОСТОЯТЕЛЬНОЕ РЕШЕНИЕ
        std::cout << "\n--- Самостоятельное решение ---\n";
        fl = 0;

        CalculusExam calculusExam;
        examScore = calculusExam.runExam(state.getPlayer());

        // Преобразуем баллы в оценку
        if (examScore >= 80) examResult = 5;
        else if (examScore >= 60) examResult = 4;
        else if (examScore >= 50) examResult = 3;
        else examResult = 2;
        break;
    }
    case 2: { // ПОМОЩЬ АЛЛЫ
        std::cout << "\n--- Помощь Аллы ---\n";

        if (allaRelation >= 60) {
            // Алла помогает -> автоматическая оценка
            if (stats.intellect >= 50) {
                fl = 1;
                examResult = 5;
                std::cout << "Алла тихонько подсказывает тебе правильные ответы.\n";
                std::cout << "Благодаря её помощи ты получаешь пять!\n";
                state.getPlayer().modifyRelation("Алла", 15);
                stats.romance = std::min(stats.romance + 8, GameConstants::MAX_STAT);
                stats.stress = std::max(stats.stress - 30, GameConstants::MIN_STAT);
            }
            else if (stats.intellect >= 40) {
                fl = 2;
                examResult = 4;
                std::cout << "Алла помогает тебе, но некоторые моменты ты всё равно не понимаешь.\n";
                std::cout << "В итоге получаешь четвёрку.\n";
                state.getPlayer().modifyRelation("Алла", 10);
                stats.romance = std::min(stats.romance + 5, GameConstants::MAX_STAT);
                stats.stress = std::max(stats.stress - 20, GameConstants::MIN_STAT);
            }
            else {
                fl = 3;
                examResult = 3;
                std::cout << "Алла изо всех сил пытается тебе помочь.\n";
                std::cout << "Но твоих знаний катастрофически не хватает.\n";
                std::cout << "Еле-еле вытягиваешь на тройку.\n";
                state.getPlayer().modifyRelation("Алла", 5);
                stats.stress = std::min(stats.stress + 5, GameConstants::MAX_STAT);
            }

            // Обновляем оценку в player
            state.getPlayer().setGrade(4, examResult * 20);
        }
        else {
            // Алла отказала
            fl = -1;
            state.getPlayer().setFlag("alla_refused_help", true);
            std::cout << "Алла отворачивается: «Извини, у меня своих проблем хватает.»\n";
            std::cout << "Приходится сдавать самостоятельно...\n";

            CalculusExam calculusExam;
            examScore = calculusExam.runExam(state.getPlayer());

            if (examScore >= 80) examResult = 5;
            else if (examScore >= 60) examResult = 4;
            else if (examScore >= 50) examResult = 3;
            else examResult = 2;

            state.getPlayer().modifyRelation("Алла", -10);
        }
        break;
    }
    case 3: { // СПИСЫВАНИЕ
        std::cout << "\n--- Списывание ---\n";
        std::cout << "Как будешь списывать?\n";
        std::cout << "1. Использовать ИИ\n";
        std::cout << "2. Списывать у другого студента\n";

        int cheatChoice;
        std::cin >> cheatChoice;
        std::cin.ignore(10000, '\n');

        if (cheatChoice == 1) {
            examResult = 2;
            fl = 0;
            std::cout << "\nНейросеть выдает полную чушь. Преподаватель замечает телефон!\n";
            std::cout << "Результат - неудовлетворительно.\n";
            stats.stress = std::min(stats.stress + 30, GameConstants::MAX_STAT);
            stats.humanity = std::max(stats.humanity - 10, GameConstants::MIN_STAT);
        }
        else {
            fl = 0;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 1);
            int luck = dis(gen);

            if (luck == 0) {
                examResult = 4;
                std::cout << "\nТебе удается списать у отличника!\n";
                std::cout << "Преподаватель ничего не замечает. Оценка 4.\n";
                stats.stress = std::max(stats.stress - 10, GameConstants::MIN_STAT);
            }
            else {
                examResult = 2;
                std::cout << "\nПреподаватель замечает, как ты вертишь головой!\n";
                std::cout << "Списывание раскрыто. Результат - неудовлетворительно.\n";
                stats.stress = std::min(stats.stress + 20, GameConstants::MAX_STAT);
            }
        }

        // Обновляем оценку в player для списывания
        state.getPlayer().setGrade(4, examResult * 20);
        break;
    }
    case 4: { // КУПИТЬ ОТВЕТЫ
        fl = 0;
        std::cout << "\n--- Покупка ответов ---\n";
        if (stats.money >= 500) {
            examResult = 5;
            stats.money -= 500;
            std::cout << "Ты покупаешь готовые ответы у старшекурсника.\n";
            std::cout << "Все решения верные, ты легко сдаешь на пять!\n";
            std::cout << "Потрачено 500 рублей.\n";
            stats.humanity = std::max(stats.humanity - 15, GameConstants::MIN_STAT);
            stats.stress = std::max(stats.stress - 25, GameConstants::MIN_STAT);

            state.getPlayer().setGrade(4, 100);
        }
        else {
            std::cout << "Недостаточно денег (нужно 500 руб.). Вариант недоступен.\n";
            std::cout << "Приходится сдавать самостоятельно...\n";

            CalculusExam calculusExam;
            examScore = calculusExam.runExam(state.getPlayer());

            if (examScore >= 80) examResult = 5;
            else if (examScore >= 60) examResult = 4;
            else if (examScore >= 50) examResult = 3;
            else examResult = 2;
        }
        break;
    }
    default:
        examResult = 3;
        break;
    }

    // Записываем оценку в нашу переменную и считаем долги
    calculusGrade = examResult;
    if (calculusGrade == 2) {
        failedExamsCount++;
    }

    // Вывод результата
    ConsoleUI::PrintHeader("РЕЗУЛЬТАТ ЭКЗАМЕНА");
    std::cout << "Твоя оценка: " << examResult << "\n";

    if (examResult >= 4) {
        std::cout << "Отличный результат! Ты горд собой.\n";
        stats.humanity = std::min(stats.humanity + 10, GameConstants::MAX_STAT);
        stats.romance = std::min(stats.romance + 5, GameConstants::MAX_STAT);
    }
    else if (examResult == 3) {
        std::cout << "Тройка. Не позор, но и не радость.\n";
    }
    else {
        std::cout << "Провал... Нужно будет пересдавать.\n";
        stats.stress = std::min(stats.stress + 20, GameConstants::MAX_STAT);
        stats.humanity = std::max(stats.humanity - 10, GameConstants::MIN_STAT);
    }

    ConsoleUI::WaitForEnter();

    // ========== ПОСЛЕ ЭКЗАМЕНА ==========
    ConsoleUI::PrintHeader("ПОСЛЕ ЭКЗАМЕНА");
    std::cout << "\nЭкзамен позади. Что будешь делать?\n";
    std::cout << "1. Сразу поехать домой\n";
    std::cout << "2. Прогуляться по университету\n";

    std::cin >> choice;
    std::cin.ignore(10000, '\n');

    // Встреча с Аллой
    if (choice == 2) {
        ConsoleUI::PrintHeader("ВСТРЕЧА С АЛЛОЙ");
        std::cout << "\nТы гуляешь по коридору и неожиданно встречаешь Аллу.\n";

        if (allaRelation >= 60) {
            std::cout << "«О, привет!» - улыбается она. «Как сдал?»\n\n";
        }
        else {
            std::cout << "Алла: «Ну что, как сдал?»\n\n";
        }

        // Диалоги в зависимости от fl
        if (fl == 1) {
            std::cout << "«У меня отлично, спасибо тебе большое!»\n";
            std::cout << "Алла покраснела: «Да не за что... Ты сам молодец!»\n";
            state.getPlayer().modifyRelation("Алла", 8);
        }
        else if (fl == 2) {
            std::cout << "«Четвёрка. Спасибо, что помогла!»\n";
            std::cout << "Алла: «Ничего страшного, главное что сдал!»\n";
            state.getPlayer().modifyRelation("Алла", 6);
        }
        else if (fl == 3) {
            std::cout << "«Тройка... Еле вытянул. Спасибо!»\n";
            std::cout << "Алла: «В следующий раз будем готовиться вместе!»\n";
            state.getPlayer().modifyRelation("Алла", 4);
        }
        else if (fl == -1) {
            if (examResult == 5) {
                std::cout << "«У меня пятерка, справился!»\n";
                std::cout << "Алла: «Ого, молодец! Извини, что не помогла.»\n";
                state.getPlayer().modifyRelation("Алла", 5);
            }
            else if (examResult == 4) {
                std::cout << "«Четверка, вытянул.»\n";
                std::cout << "Алла: «Неплохо! Извини.»\n";
                state.getPlayer().modifyRelation("Алла", 3);
            }
            else if (examResult == 3) {
                std::cout << "«Тройка, еле-еле.»\n";
                std::cout << "Алла: «Главное сдал.»\n";
                state.getPlayer().modifyRelation("Алла", 2);
            }
            else {
                std::cout << "«Провалил...»\n";
                std::cout << "Алла виновато смотрит: «Прости...»\n";
                state.getPlayer().modifyRelation("Алла", -5);
            }
        }
        else {
            if (examResult == 5) {
                std::cout << "«У меня пятерка! А у тебя?»\n";
                if (allaRelation >= 60) std::cout << "Алла: «У меня тоже!»\n";
                else std::cout << "Алла: «У меня тоже.»\n";
                state.getPlayer().modifyRelation("Алла", 5);
            }
            else if (examResult == 4) {
                std::cout << "«Хорошо! А у тебя?»\n";
                if (allaRelation >= 60) std::cout << "Алла: «У меня отлично!»\n";
                else std::cout << "Алла: «У меня пятерка.»\n";
                state.getPlayer().modifyRelation("Алла", 5);
            }
            else if (examResult == 3) {
                std::cout << "«На троечку, но главное - сдал.»\n";
                if (allaRelation >= 60) std::cout << "Алла: «Молодец!»\n";
                else std::cout << "Алла: «Главное - сдал!»\n";
                state.getPlayer().modifyRelation("Алла", 3);
            }
            else {
                std::cout << "«Провалил... Придется пересдавать.»\n";
                if (allaRelation >= 60) std::cout << "Алла: «Держись!»\n";
                else std::cout << "Алла: «Ничего, пересдашь.»\n";
                state.getPlayer().modifyRelation("Алла", 2);
            }
        }

        ConsoleUI::WaitForEnter();

        // Приглашение на свидание
        if (allaRelation >= 60) {
            ConsoleUI::PrintHeader("ПРИГЛАШЕНИЕ НА СВИДАНИЕ");
            std::cout << "Ты чувствуешь, что сейчас подходящий момент.\n";
            std::cout << "1. Позвать Аллу на свидание\n";
            std::cout << "2. Попрощаться и уйти домой\n";

            std::cin >> choice;
            std::cin.ignore(10000, '\n');

            if (choice == 1) {
                if (allaRelation >= 65) {
                    std::cout << "\nАлла краснеет: «Правда? Я... да, с удовольствием!»\n";
                    std::cout << "Свидание назначено!\n";
                    state.getPlayer().setFlag("date_with_alla_scheduled", true);
                    stats.romance = std::min(stats.romance + 20, GameConstants::MAX_STAT);
                    state.getPlayer().modifyRelation("Алла", 15);
                    std::cout << "\nОткрыта дополнительная сюжетная линия!\n";
                }
                else {
                    std::cout << "\nАлла: «Извини, я не готова...»\n";
                    state.getPlayer().modifyRelation("Алла", -5);
                    state.getPlayer().setFlag("alla_refused_help", true);
                }
            }
        }
        else {
            std::cout << "\nВы немного обсуждаете экзамены и расходитесь.\n";
        }
    }
    else {
        std::cout << "\nТы едешь домой, уставший после экзамена.\n";
    }

    // Обновление параметров в конце дня
    stats.fatigue = std::min(stats.fatigue + 15, GameConstants::MAX_FATIGUE);
    stats.hunger = std::max(stats.hunger - 10, 0);
    if (examResult == 2) {
        stats.stress = std::min(stats.stress + 15, GameConstants::MAX_STAT);
    }
    stats.clampAll();

    ConsoleUI::PrintHeader("КОНЕЦ ДНЯ 5");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}
// ==================== ДЕНЬ 6 — ВЫХОДНОЙ(Подготовка к свиданию, если договорился) ====================
void Game::runDay6() {
    // Сбрасываем флаг звонка на новый день
    state.getPlayer().setFlag("called_alla_day6", false);
    ConsoleUI::PrintDayHeader(6, "Выходной");
    std::cout << "\nНаконец-то выходной! Можно отдохнуть.\n";
    std::cout << "Сегодня нет экзаменов. Ты сам решаешь, чем заняться.\n";
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();
    bool continueDay = true;
    bool hasDoneAction = false;
    bool preparationDone = false;

    // Получаем информацию из дня 5
    bool hasDateScheduled = state.getPlayer().hasFlag("date_with_alla_scheduled");
    bool allaRefused = state.getPlayer().hasFlag("alla_refused_help");

    // Флаги для подарков
    bool hasFlowers = false;
    bool hasCandies = false;

    int allaRelation = state.getPlayer().getRelation("Алла");

    // ========== ПОДГОТОВКА К СВИДАНИЮ (если оно запланировано) ==========
    if (hasDateScheduled) {
        ConsoleUI::PrintHeader("ПОДГОТОВКА К СВИДАНИЮ");
        std::cout << "\nТы вспоминаешь, что завтра у тебя свидание с Аллой!\n";
        std::cout << "Нужно подготовиться: купить цветы и конфеты.\n\n";

        std::cout << "Что купить для свидания?\n";
        std::cout << "1. Только цветы (300 руб.)\n";
        std::cout << "2. Только конфеты (250 руб.)\n";
        std::cout << "3. И то, и другое (550 руб.)\n";
        std::cout << "4. Ничего, пойду с пустыми руками\n";

        int prepChoice;
        std::cin >> prepChoice;
        std::cin.ignore(10000, '\n');

        switch (prepChoice) {
        case 1:
            if (stats.money >= 300) {
                stats.money -= 300;
                hasFlowers = true;
                std::cout << "\nТы купил красивый букет роз!\n";
                std::cout << "-300 руб\n";
            }
            else {
                std::cout << "\nНедостаточно денег для цветов.\n";
            }
            break;
        case 2:
            if (stats.money >= 250) {
                stats.money -= 250;
                hasCandies = true;
                std::cout << "\nТы купил коробку конфет!\n";
                std::cout << "-250 руб\n";
            }
            else {
                std::cout << "\nНедостаточно денег для конфет.\n";
            }
            break;
        case 3:
            if (stats.money >= 550) {
                stats.money -= 550;
                hasFlowers = true;
                hasCandies = true;
                std::cout << "\nТы купил цветы и конфеты!\n";
                std::cout << "-550 руб\n";
            }
            else {
                std::cout << "\nНедостаточно денег для покупок.\n";
            }
            break;
        default:
            std::cout << "\nТы решил идти с пустыми руками.\n";
            break;
        }

        ConsoleUI::WaitForEnter();

        // Выход на улицу для дополнительных покупок
        bool shoppingDone = false;
        while (!shoppingDone) {
            ConsoleUI::ClearScreen();
            ConsoleUI::PrintHeader("ПРОГУЛКА ПО УЛИЦЕ");
            ConsoleUI::PrintPlayerStats(state.getPlayer());

            std::cout << "\nГде ты находишься?\n";
            std::cout << "1. Цветочный магазин\n";
            std::cout << "2. Продуктовый магазин\n";
            std::cout << "3. Вернуться домой\n";

            int shopChoice;
            std::cin >> shopChoice;
            std::cin.ignore(10000, '\n');

            if (shopChoice == 1) {
                ConsoleUI::PrintHeader("ЦВЕТОЧНЫЙ МАГАЗИН");
                std::cout << "\nВ магазине красивые розы, тюльпаны и ромашки.\n";
                std::cout << "1. Купить цветы (300 руб.)\n";
                std::cout << "2. Не покупать цветы\n";

                int flowerChoice;
                std::cin >> flowerChoice;
                std::cin.ignore(10000, '\n');

                if (flowerChoice == 1 && stats.money >= 300 && !hasFlowers) {
                    stats.money -= 300;
                    hasFlowers = true;
                    std::cout << "\nТы купил красивый букет!\n";
                }
                else if (flowerChoice == 1 && stats.money < 300) {
                    std::cout << "\nНедостаточно денег.\n";
                }
                else if (flowerChoice == 1 && hasFlowers) {
                    std::cout << "\nУ тебя уже есть цветы.\n";
                }
                ConsoleUI::WaitForEnter();
            }
            else if (shopChoice == 2) {
                ConsoleUI::PrintHeader("ПРОДУКТОВЫЙ МАГАЗИН");
                std::cout << "\nВ магазине большой выбор конфет.\n";
                std::cout << "1. Купить конфеты (250 руб.)\n";
                std::cout << "2. Не покупать конфеты\n";

                int candyChoice;
                std::cin >> candyChoice;
                std::cin.ignore(10000, '\n');

                if (candyChoice == 1 && stats.money >= 250 && !hasCandies) {
                    stats.money -= 250;
                    hasCandies = true;
                    std::cout << "\nТы купил коробку конфет!\n";
                }
                else if (candyChoice == 1 && stats.money < 250) {
                    std::cout << "\nНедостаточно денег.\n";
                }
                else if (candyChoice == 1 && hasCandies) {
                    std::cout << "\nУ тебя уже есть конфеты.\n";
                }
                ConsoleUI::WaitForEnter();
            }
            else if (shopChoice == 3) {
                shoppingDone = true;
            }

            // Случайные события на улице
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 100);
            int randomEvent = dis(gen);

            if (randomEvent < 30) {
                ConsoleUI::PrintHeader("СЛУЧАЙНОЕ СОБЫТИЕ");
                std::uniform_int_distribution<> eventDis(1, 5);
                int eventType = eventDis(gen);

                switch (eventType) {
                case 1:
                    std::cout << "\nТы нашёл на улице 100 рублей!\n";
                    stats.money += 100;
                    break;
                case 2:
                    std::cout << "\nТы встретил знакомого. Вы немного поболтали.\n";
                    stats.stress = std::max(stats.stress - 5, GameConstants::MIN_STAT);
                    break;
                case 3:
                    std::cout << "\nТы купил еду со скидкой.\n";
                    stats.hunger = std::min(stats.hunger + 15, GameConstants::MAX_HUNGER);
                    stats.money -= 50;
                    break;
                case 4:
                    std::cout << "\nТы потерял 50 рублей.\n";
                    stats.money = std::max(stats.money - 50, 0);
                    stats.stress = std::min(stats.stress + 5, GameConstants::MAX_STAT);
                    break;
                case 5:
                    if (randomEvent % 2 == 0) {
                        std::cout << "\nТы встретил Булата! Вы немного поболтали.\n";
                        state.getPlayer().modifyRelation("Булат", 3);
                    }
                    else {
                        std::cout << "\nТы встретил Семёна! Вы немного поболтали.\n";
                        state.getPlayer().modifyRelation("Семён", 3);
                    }
                    stats.stress = std::max(stats.stress - 8, GameConstants::MIN_STAT);
                    break;
                }
                ConsoleUI::WaitForEnter();
            }
        }

        preparationDone = true;
        std::cout << "\nПодготовка завершена. Ты вернулся домой.\n";
        ConsoleUI::WaitForEnter();

        // Сохраняем флаги подарков
        state.getPlayer().setFlag("has_flowers", hasFlowers);
        state.getPlayer().setFlag("has_candies", hasCandies);
    }

    // ========== ОСНОВНЫЕ ДЕЙСТВИЯ ДНЯ ==========

    while (continueDay) {
        ConsoleUI::ClearScreen();
        ConsoleUI::PrintDayHeader(6, "Выходной");
        ConsoleUI::PrintPlayerStats(state.getPlayer());

        std::cout << "\n------------------------------------------------------------\n";
        std::cout << "Чем хочешь заняться?\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "1. Пойти работать\n";
        std::cout << "2. Остаться дома\n";
        std::cout << "3. Играть в компьютер\n";

        if (!hasDateScheduled) {
            std::cout << "4. Позвонить Алле\n";
        }

        if (hasDoneAction || preparationDone) {
            std::cout << "5. Закончить день\n";
        }
        else {
            std::cout << "5. (сначала сделай что-нибудь)\n";
        }
        std::cout << "------------------------------------------------------------\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');
        if (choice == 5 && !hasDoneAction && !preparationDone) {
            std::cout << "\nТы только проснулся. Сделай хотя бы что-нибудь!\n";
            ConsoleUI::WaitForEnter();
            continue;
        }

        switch (choice) {
        case 1: {
            ConsoleUI::PrintHeader("РАБОТА");
            std::cout << "\nТы идёшь на подработку в кафе.\n";
            std::cout << "День прошёл тяжело, но ты заработал деньги.\n";

            stats.money += 800;
            stats.energy = std::max(stats.energy - 30, GameConstants::MIN_STAT);
            stats.fatigue = std::min(stats.fatigue + 25, GameConstants::MAX_FATIGUE);
            stats.stress = std::min(stats.stress + 15, GameConstants::MAX_STAT);
            stats.hunger = std::max(stats.hunger - 20, GameConstants::MIN_STAT);  // сытость уменьшается

            std::cout << "\n+800 руб\n";
            std::cout << "-Энергия\n";
            std::cout << "+Усталость\n";
            std::cout << "+Стресс\n";

            state.getPlayer().advanceTime(480);
            hasDoneAction = true;
            ConsoleUI::WaitForEnter();
            break;
        }
        case 2: {
            ConsoleUI::PrintHeader("ДОМАШНИЙ ОТДЫХ");
            std::cout << "\nТы решил провести день дома:\n";
            std::cout << "- Поспал до обеда\n";
            std::cout << "- Посмотрел интересный фильм\n";
            std::cout << "- Почитал учебник\n";

            stats.energy = std::min(stats.energy + 40, GameConstants::MAX_STAT);
            stats.fatigue = std::max(stats.fatigue - 30, GameConstants::MIN_STAT);
            stats.intellect = std::min(stats.intellect + 3, GameConstants::MAX_STAT);
            stats.stress = std::max(stats.stress - 20, GameConstants::MIN_STAT);

            std::cout << "\n+Энергия\n";
            std::cout << "-Усталость\n";
            std::cout << "+Интеллект\n";
            std::cout << "-Стресс\n";

            state.getPlayer().advanceTime(360);
            hasDoneAction = true;
            ConsoleUI::WaitForEnter();
            break;
        }
        case 3: {
            ConsoleUI::PrintHeader("КОМПЬЮТЕРНЫЕ ИГРЫ");
            std::cout << "\nТы целый день просидел за компьютером:\n";
            std::cout << "- Прошёл сложную игру\n";
            std::cout << "- Поболтал с друзьями\n";

            stats.stress = std::max(stats.stress - 25, GameConstants::MIN_STAT);
            stats.energy = std::max(stats.energy - 15, GameConstants::MIN_STAT);
            stats.intellect = std::max(stats.intellect - 2, GameConstants::MIN_STAT);

            std::cout << "\n-Стресс\n";
            std::cout << "-Энергия\n";
            std::cout << "-Интеллект\n";

            state.getPlayer().advanceTime(300);
            hasDoneAction = true;
            ConsoleUI::WaitForEnter();
            break;
        }
        case 4: {
            if (hasDateScheduled) {
                std::cout << "\nТы уже готовишься к завтрашнему свиданию.\n";
                std::cout << "Не стоит звонить Алле сейчас.\n";
                ConsoleUI::WaitForEnter();
                break;
            }

            // Проверяем, звонил ли уже сегодня
            if (state.getPlayer().hasFlag("called_alla_day6")) {
                std::cout << "\nТы уже звонил Алле сегодня.\n";
                std::cout << "Не стоит надоедать.\n";
                ConsoleUI::WaitForEnter();
                break;
            }

            ConsoleUI::PrintHeader("ЗВОНОК АЛЛЕ");
            std::cout << "\nТы набираешь номер Аллы...\n";
            std::cout << "Гудок... Гудок...\n";

            // Проверяем, отказывала ли Алла в помощи (из дня 5)
            bool allaRefused = state.getPlayer().hasFlag("alla_refused_help");
            // Проверяем, приглашал ли он её на свидание
            bool invitedToDate = state.getPlayer().hasFlag("invited_alla_to_walk");

            if (allaRefused) {
                // СЦЕНАРИЙ: Алла отказала в помощи, извиняемся
                std::cout << "\n«Алло...» - раздаётся голос Аллы.\n";
                std::cout << "Ты: «Алла, привет. Я хотел извиниться за вчерашнее.\n";
                std::cout << "Я был неправ, что не помог. Может, погуляем сегодня?»\n";
                std::cout << "Алла: «Ладно... Давай встретимся через час в парке.»\n";

                state.getPlayer().modifyRelation("Алла", 10);
                stats.romance = std::min(stats.romance + 5, GameConstants::MAX_STAT);

                ConsoleUI::WaitForEnter();

                std::cout << "\nВы гуляете по парку, обсуждаете учёбу.\n";
                std::cout << "Алла не держит зла.\n";
                std::cout << "\n+Отношения с Аллой\n";
                std::cout << "-Стресс\n";
                state.getPlayer().modifyRelation("Алла", 5);
                stats.stress = std::max(stats.stress - 15, GameConstants::MIN_STAT);

                state.getPlayer().setFlag("alla_refused_help", false);
            }
            else if (!invitedToDate && allaRelation >= 60) {
                // СЦЕНАРИЙ: не приглашал на свидание, но отношения хорошие
                std::cout << "\n«Алло! Привет!» - радостно отвечает Алла.\n";
                std::cout << "Ты: «Алла, хочешь погулять сегодня?»\n";
                std::cout << "Алла: «С удовольствием! А то я сижу дома скучаю.»\n";

                state.getPlayer().modifyRelation("Алла", 5);
                stats.romance = std::min(stats.romance + 5, GameConstants::MAX_STAT);

                ConsoleUI::WaitForEnter();

                std::cout << "\nВы идёте в кафе, потом гуляете по городу.\n";
                std::cout << "Алла: «Спасибо, что позвал. Мне было грустно одной.»\n";
                std::cout << "\n+Отношения с Аллой\n";
                std::cout << "+Романтика\n";
                std::cout << "-Стресс\n";
                state.getPlayer().modifyRelation("Алла", 5);
                stats.romance = std::min(stats.romance + 3, GameConstants::MAX_STAT);
                stats.stress = std::max(stats.stress - 20, GameConstants::MIN_STAT);

                state.getPlayer().setFlag("invited_alla_to_walk", true);
            }
            else if (invitedToDate && allaRelation >= 70) {
                // СЦЕНАРИЙ: уже приглашал на свидание (день 7)
                std::cout << "\n«Алло! Привет!» - улыбается Алла.\n";
                std::cout << "Ты: «Привет! Как дела? Завтра наш день.»\n";
                std::cout << "Алла: «Да, я жду не дождусь! Уже всё продумала.»\n";
                std::cout << "Ты: «Тогда до завтра!»\n";
                std::cout << "Алла: «До завтра! Буду наряжаться.»\n";

                state.getPlayer().modifyRelation("Алла", 3);
                stats.romance = std::min(stats.romance + 2, GameConstants::MAX_STAT);
                stats.stress = std::max(stats.stress - 5, GameConstants::MIN_STAT);

                ConsoleUI::WaitForEnter();
            }
            else if (allaRelation >= 50) {
                // СЦЕНАРИЙ: обычный разговор
                std::cout << "\n«Алло! Привет!» - улыбается Алла.\n";
                std::cout << "Ты: «Погуляем сегодня?»\n";
                std::cout << "Алла: «Давай! Мне как раз нужно отвлечься от учёбы.»\n";

                state.getPlayer().modifyRelation("Алла", 4);
                stats.romance = std::min(stats.romance + 3, GameConstants::MAX_STAT);

                ConsoleUI::WaitForEnter();

                std::cout << "\nВы гуляете по парку, кормите уток.\n";
                std::cout << "\n+Отношения с Аллой\n";
                std::cout << "-Стресс\n";
                state.getPlayer().modifyRelation("Алла", 4);
                stats.stress = std::max(stats.stress - 20, GameConstants::MAX_STAT);
            }
            else {
                // СЦЕНАРИЙ: плохие отношения
                std::cout << "\n«Абонент временно недоступен. Перезвоните позже.»\n";
                std::cout << "Странно... Алла не берёт трубку.\n";
                std::cout << "Возможно, она обиделась на что-то.\n";
                std::cout << "\n-Отношения с Аллой\n";
                state.getPlayer().modifyRelation("Алла", -3);
            }

            // Помечаем, что звонок был сделан
            state.getPlayer().setFlag("called_alla_day6", true);
            hasDoneAction = true;
            ConsoleUI::WaitForEnter();
            break;
        }
        case 5:
            if (hasDoneAction || preparationDone) {
                std::cout << "\nТы решил, что на сегодня достаточно.\n";
                ConsoleUI::WaitForEnter();
                continueDay = false;
            }
            else {
                std::cout << "\nТы только проснулся. Сделай хотя бы что-нибудь!\n";
                ConsoleUI::WaitForEnter();
            }
            break;
        default:
            std::cout << "\nНеверный выбор.\n";
            ConsoleUI::WaitForEnter();
            break;
        }

        stats.clampAll();
        checkGameOver();
        if (state.getPhase() != GamePhase::Playing) return;
    }

    // Случайные события в конце дня
    eventManager.tryTriggerEvent(state.getPlayer(), 6);

    // ========== ВЕЧЕРНИЙ РИТУАЛ ==========
    if (hasDoneAction || preparationDone) {
        ConsoleUI::PrintHeader("ВЕЧЕР");
        std::cout << "\nВечер подходит к концу.\n";
        ConsoleUI::WaitForEnter();

        ConsoleUI::PrintHeader("НА КУХНЕ");
        std::cout << "\nТы заходишь на кухню и ужинаешь.\n";
        std::cout << "\n+Сытость\n";
        stats.hunger = std::min(stats.hunger + 15, GameConstants::MAX_HUNGER);
        ConsoleUI::WaitForEnter();

        ConsoleUI::PrintHeader("СОН");
        std::cout << "\nТы ложишься в кровать и закрываешь глаза.\n";
        std::cout << "Мысли о прошедшем дне медленно угасают...\n";
        std::cout << "Ты засыпаешь, набираясь сил перед новым днём.\n";
        std::cout << "\n+Энергия\n";
        std::cout << "-Стресс\n";
        stats.energy = std::min(stats.energy + 10, GameConstants::MAX_STAT);
        stats.stress = std::max(stats.stress - 5, GameConstants::MIN_STAT);
        ConsoleUI::WaitForEnter();
    }
    else {
        ConsoleUI::PrintHeader("ДОМАШНИЙ ВЕЧЕР");
        std::cout << "\nТы провёл весь день дома, отдыхая.\n";
        std::cout << "\n+Энергия\n";
        std::cout << "-Стресс\n";
        stats.energy = std::min(stats.energy + 20, GameConstants::MAX_STAT);
        stats.stress = std::max(stats.stress - 15, GameConstants::MIN_STAT);
        ConsoleUI::WaitForEnter();
    }

    ConsoleUI::PrintHeader("СТАТИСТИКА ДНЯ 6");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}
// ==================== ДЕНЬ 7 — СВИДАНИЕ(Возможность пересдать 1 предмет или обычный выходной) ====================
void Game::runDay7() {
    ConsoleUI::PrintDayHeader(7, "Седьмой день");
    std::cout << "\nНаступил новый день.\n";
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();
    bool hasDateScheduled = state.getPlayer().hasFlag("date_with_alla_scheduled");
    bool hasFlowers = state.getPlayer().hasFlag("has_flowers");
    bool hasCandies = state.getPlayer().hasFlag("has_candies");
    int allaRelation = state.getPlayer().getRelation("Алла");

    // ========== ПРОВЕРКА: ЕСТЬ ЛИ СВИДАНИЕ И ДОЛГИ? ==========

    // СЦЕНАРИЙ 1: Есть и свидание, и долги
    if (hasDateScheduled && failedExamsCount > 0) {
        ConsoleUI::PrintHeader("ВЫБОР");
        std::cout << "\nУ тебя сегодня запланировано свидание с Аллой!\n";
        std::cout << "Но у тебя есть " << failedExamsCount << " долг(а/ов).\n";
        std::cout << "Тебе нужно готовиться к пересдаче.\n\n";
        std::cout << "Что для тебя важнее?\n";
        std::cout << "1. Пойти на свидание (долги останутся)\n";
        std::cout << "2. Пойти на пересдачу (свидание отменяется)\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        if (choice == 1) {
            std::cout << "\nТы решил, что отношения важнее учёбы.\n";
            runDateWithAlla(hasFlowers, hasCandies, allaRelation, stats);
            state.getPlayer().setFlag("date_with_alla_scheduled", false);
        }
        else {
            std::cout << "\nТы решил, что учёба важнее.\n";
            std::cout << "Ты набираешь номер Аллы...\n";
            std::cout << "«Алло?» - раздаётся голос Аллы.\n";
            std::cout << "«Извини, Алл, у меня пересдача, я не смогу прийти на свидание...»\n";
            std::cout << "Алла холодно отвечает: «Хорошо. Удачи.»\n";
            std::cout << "*гудки*\n";
            state.getPlayer().modifyRelation("Алла", -5);
            state.getPlayer().setFlag("date_with_alla_scheduled", false);
            runRetakeExam(stats);
        }
    }
    // СЦЕНАРИЙ 2: Только свидание (долгов нет)
    else if (hasDateScheduled && failedExamsCount == 0) {
        ConsoleUI::PrintHeader("СВИДАНИЕ");
        std::cout << "\nСегодня у тебя свидание с Аллой!\n";
        runDateWithAlla(hasFlowers, hasCandies, allaRelation, stats);
        state.getPlayer().setFlag("date_with_alla_scheduled", false);
    }
    // СЦЕНАРИЙ 3: Только долги (свидания нет)
    else if (!hasDateScheduled && failedExamsCount > 0) {
        ConsoleUI::PrintHeader("ПЕРЕСДАЧА");
        std::cout << "\nУ тебя есть " << failedExamsCount << " долг(а/ов).\n";
        std::cout << "Сегодня последний шанс пересдать экзамены.\n";
        runRetakeExam(stats);
    }
    // СЦЕНАРИЙ 4: Ничего нет (обычный день)
    else {
        runNormalDay();
    }

    // Случайные события
    eventManager.tryTriggerEvent(state.getPlayer(), 7);

    // Финальная статистика дня
    ConsoleUI::PrintHeader("СТАТИСТИКА ДНЯ 7");
    ConsoleUI::PrintPlayerStats(state.getPlayer());
    ConsoleUI::WaitForEnter();
}

// ========== СВИДАНИЕ С АЛЛОЙ (полная версия) ==========
void Game::runDateWithAlla(bool hasFlowers, bool hasCandies, int allaRelation, Stats& stats) {
    // 7.1) Перед выходом - проверка подарков
    ConsoleUI::PrintHeader("ПОДГОТОВКА К ВЫХОДУ");
    std::cout << "\nТы готовишься к выходу. Проверяешь, что купил:\n";
    std::cout << "- Цветы: " << (hasFlowers ? "Есть" : "Нет") << "\n";
    std::cout << "- Конфеты: " << (hasCandies ? "Есть" : "Нет") << "\n";
    ConsoleUI::WaitForEnter();

    // 7.2) Встреча возле университета
    ConsoleUI::PrintHeader("ВСТРЕЧА");
    std::cout << "\nТы встречаешь Аллу возле университета.\n";
    std::cout << "Она улыбается и машет рукой.\n";
    std::cout << "«Привет! Я так рада, что мы решили встретиться!»\n";
    ConsoleUI::WaitForEnter();

    // 7.2.1) Выбор подарка
    ConsoleUI::PrintHeader("ПОДАРКИ");
    std::cout << "\nЧто подаришь Алле?\n";

    int giftChoice = 0;
    if (hasFlowers && hasCandies) {
        std::cout << "1. Подарить цветы\n";
        std::cout << "2. Подарить конфеты\n";
        std::cout << "3. Подарить и то, и другое\n";
        std::cout << "4. Не дарить ничего\n";

        std::cin >> giftChoice;
        std::cin.ignore(10000, '\n');

        switch (giftChoice) {
        case 1:
            std::cout << "\nТы даришь Алле цветы.\n";
            std::cout << "«Ой, какие красивые! Спасибо тебе большое!»\n";
            state.getPlayer().modifyRelation("Алла", 10);
            stats.romance = std::min(stats.romance + 10, GameConstants::MAX_STAT);
            break;
        case 2:
            std::cout << "\nТы даришь Алле конфеты.\n";
            std::cout << "«Ты знаешь, это мои любимые! Как ты догадался?»\n";
            state.getPlayer().modifyRelation("Алла", 8);
            stats.romance = std::min(stats.romance + 8, GameConstants::MAX_STAT);
            break;
        case 3:
            std::cout << "\nТы даришь Алле цветы и коробку конфет.\n";
            std::cout << "Алла ахнула: «Ой! Это всё мне? Ты не должен был тратиться!»\n";
            std::cout << "Она смущённо улыбается, её щёки розовеют.\n";
            state.getPlayer().modifyRelation("Алла", 15);
            stats.romance = std::min(stats.romance + 15, GameConstants::MAX_STAT);
            break;
        default:
            std::cout << "\nТы пришёл с пустыми руками.\n";
            break;
        }
    }
    else if (hasFlowers) {
        std::cout << "1. Подарить цветы\n";
        std::cout << "2. Не дарить ничего\n";

        std::cin >> giftChoice;
        std::cin.ignore(10000, '\n');

        if (giftChoice == 1) {
            std::cout << "\nТы даришь Алле цветы.\n";
            std::cout << "«Ой, какие красивые! Спасибо тебе большое!»\n";
            state.getPlayer().modifyRelation("Алла", 10);
            stats.romance = std::min(stats.romance + 10, GameConstants::MAX_STAT);
        }
        else {
            std::cout << "\nТы пришёл с пустыми руками.\n";
        }
    }
    else if (hasCandies) {
        std::cout << "1. Подарить конфеты\n";
        std::cout << "2. Не дарить ничего\n";

        std::cin >> giftChoice;
        std::cin.ignore(10000, '\n');

        if (giftChoice == 1) {
            std::cout << "\nТы даришь Алле конфеты.\n";
            std::cout << "«Ты знаешь, это мои любимые! Как ты догадался?»\n";
            state.getPlayer().modifyRelation("Алла", 8);
            stats.romance = std::min(stats.romance + 8, GameConstants::MAX_STAT);
        }
        else {
            std::cout << "\nТы пришёл с пустыми руками.\n";
        }
    }
    else {
        std::cout << "1. Не дарить ничего\n";
        std::cin >> giftChoice;
        std::cin.ignore(10000, '\n');
        std::cout << "\nТы пришёл с пустыми руками.\n";
    }

    ConsoleUI::WaitForEnter();

    // 7.3) Прогулка и разговор
    ConsoleUI::PrintHeader("ПРОГУЛКА");
    std::cout << "\nВы гуляете по городу. Алла улыбается и наслаждается компанией.\n";
    ConsoleUI::WaitForEnter();

    ConsoleUI::PrintHeader("РАЗГОВОР");
    std::cout << "\nАлла: «Расскажи что-нибудь о себе. Чем ты увлекаешься?»\n";
    std::cout << "Что будешь говорить?\n";
    std::cout << "1. Поговорить об учёбе\n";
    std::cout << "2. Поговорить об увлечениях Аллы\n";
    std::cout << "3. Постоянно говорить о себе\n";
    std::cout << "4. Спросить о её планах после университета\n";

    int talkChoice;
    std::cin >> talkChoice;
    std::cin.ignore(10000, '\n');

    switch (talkChoice) {
    case 1:
        std::cout << "\nТы рассказываешь о своих успехах в учёбе.\n";
        std::cout << "Алла слушает вежливо, но не очень заинтересованно.\n";
        state.getPlayer().modifyRelation("Алла", 3);
        break;
    case 2:
        std::cout << "\nТы спрашиваешь Аллу о её хобби.\n";
        std::cout << "Она оживляется и рассказывает о рисовании.\n";
        state.getPlayer().modifyRelation("Алла", 8);
        break;
    case 3:
        std::cout << "\nТы всё время говоришь только о себе.\n";
        std::cout << "Алла выглядит слегка разочарованной.\n";
        state.getPlayer().modifyRelation("Алла", -5);
        break;
    case 4:
        std::cout << "\nТы спрашиваешь о её планах на будущее.\n";
        std::cout << "Алла задумывается и делится мечтами.\n";
        state.getPlayer().modifyRelation("Алла", 12);
        stats.romance = std::min(stats.romance + 5, GameConstants::MAX_STAT);
        break;
    default:
        break;
    }

    ConsoleUI::WaitForEnter();

    // 7.4) Случайное событие - дождь
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    int rainChance = dis(gen);

    if (rainChance <= 30) {
        ConsoleUI::PrintHeader("НЕОЖИДАННЫЙ ДОЖДЬ");
        std::cout << "\nНеожиданно начинается дождь. Капли барабанят по листьям.\n";
        std::cout << "Алла: «Ой, кажется, мы не взяли зонты...»\n";
        std::cout << "Что делать?\n";
        std::cout << "1. Проводить Аллу домой\n";
        std::cout << "2. Попрощаться и уйти\n";

        int rainChoice;
        std::cin >> rainChoice;
        std::cin.ignore(10000, '\n');

        if (rainChoice == 1) {
            std::cout << "\nТы проводил Аллу до дома. Она благодарна.\n";
            state.getPlayer().modifyRelation("Алла", 5);
            stats.romance = std::min(stats.romance + 3, GameConstants::MAX_STAT);
        }
        else {
            std::cout << "\nВы попрощались и разбежались по домам.\n";
        }
        ConsoleUI::WaitForEnter();
    }

    // 7.5) Конец свидания
    ConsoleUI::PrintHeader("ПРОЩАНИЕ");
    std::cout << "\nСвидание подходит к концу. Алла смотрит на тебя с надеждой.\n";
    std::cout << "Алла: «Сегодня был чудесный день...»\n";
    std::cout << "Что скажешь?\n";
    std::cout << "1. «Мне было приятно провести с тобой время»\n";
    std::cout << "2. «Надо будет ещё как-нибудь встретиться»\n";
    std::cout << "3. Попрощаться без дополнительных слов\n";

    int finalChoice;
    std::cin >> finalChoice;
    std::cin.ignore(10000, '\n');

    switch (finalChoice) {
    case 1:
        std::cout << "\n«Мне тоже было приятно» - улыбается Алла.\n";
        state.getPlayer().modifyRelation("Алла", 5);
        break;
    case 2:
        std::cout << "\n«Я буду ждать!» - радостно отвечает Алла.\n";
        state.getPlayer().modifyRelation("Алла", 10);
        stats.romance = std::min(stats.romance + 8, GameConstants::MAX_STAT);
        break;
    default:
        std::cout << "\nВы молча расходитесь.\n";
        break;
    }

    ConsoleUI::WaitForEnter();

    // 7.6) Итог свидания
    int finalRelation = state.getPlayer().getRelation("Алла");

    ConsoleUI::PrintHeader("ИТОГ СВИДАНИЯ");

    if (finalRelation >= 80) {
        std::cout << "\nАлла берёт тебя за руку и говорит:\n";
        std::cout << "«Ты знаешь... Мне очень нравится с тобой проводить время.\n";
        std::cout << "Ты особенный человек. Может, встретимся ещё?»\n";
        state.getPlayer().setFlag("romantic_ending", true);
        stats.romance = std::min(stats.romance + 20, GameConstants::MAX_STAT);
        std::cout << "\nОткрыта счастливая концовка с Аллой!\n";
    }
    else if (finalRelation >= 60) {
        std::cout << "\n«Спасибо за прекрасный день! Давай ещё встретимся как-нибудь»\n";
        std::cout << "Алла остаётся хорошей подругой.\n";
    }
    else {
        std::cout << "\nАлла быстро уходит, не оборачиваясь.\n";
        std::cout << "Что-то пошло не так... Романтическая концовка стала недоступна.\n";
    }

    ConsoleUI::WaitForEnter();
}
// ========== ПЕРЕСДАЧА ЭКЗАМЕНА ==========
void Game::runRetakeExam(Stats& stats) {
    ConsoleUI::PrintHeader("ПЕРЕСДАЧА ЭКЗАМЕНА");
    std::cout << "\nСегодня последний шанс пересдать  один экзамен.\n";
    ConsoleUI::WaitForEnter();

    ConsoleUI::PrintHeader("ВЫБОР ЭКЗАМЕНА");
    std::cout << "Какой экзамен будешь пересдавать?\n";

    std::cout << "1. История";
    if (historyGrade == 2) std::cout << " (есть долг)\n";
    else if (historyGrade >= 3) std::cout << " (сдан)\n";
    else std::cout << " (не сдавался)\n";

    std::cout << "2. Языки и методы программирования";
    if (yampGrade == 2) std::cout << " (есть долг)\n";
    else if (yampGrade >= 3) std::cout << " (сдан)\n";
    else std::cout << " (не сдавался)\n";

    std::cout << "3. Дискретная математика";
    if (discreteGrade == 2) std::cout << " (есть долг)\n";
    else if (discreteGrade >= 3) std::cout << " (сдан)\n";
    else std::cout << " (не сдавался)\n";

    std::cout << "4. Математический анализ";
    if (calculusGrade == 2) std::cout << " (есть долг)\n";
    else if (calculusGrade >= 3) std::cout << " (сдан)\n";
    else std::cout << " (не сдавался)\n";

    std::cout << "0. Отказаться от пересдачи\n";

    int examChoice;
    std::cin >> examChoice;
    std::cin.ignore(10000, '\n');

    int selectedExamId = 0;
    std::string selectedExamName = "";

    if (examChoice == 1 && historyGrade == 2) {
        selectedExamId = 1;
        selectedExamName = "История";
    }
    else if (examChoice == 2 && yampGrade == 2) {
        selectedExamId = 2;
        selectedExamName = "Языки и методы программирования";
    }
    else if (examChoice == 3 && discreteGrade == 2) {
        selectedExamId = 4;
        selectedExamName = "Дискретная математика";
    }
    else if (examChoice == 4 && calculusGrade == 2) {
        selectedExamId = 5;
        selectedExamName = "Математический анализ";
    }

    if (examChoice >= 1 && examChoice <= 4 && selectedExamId != 0) {
        ConsoleUI::PrintHeader("ПОДГОТОВКА К ПЕРЕСДАЧЕ");
        std::cout << "\nКак будешь готовиться к пересдаче " << selectedExamName << "?\n";
        std::cout << "1. Интенсивно заниматься\n";
        std::cout << "2. Повторить основные темы\n";
        std::cout << "3. Надеяться на удачу\n";

        int prepChoice;
        std::cin >> prepChoice;
        std::cin.ignore(10000, '\n');

        int intellectBonus = 0;
        switch (prepChoice) {
        case 1:
            intellectBonus = 10;
            stats.intellect = std::min(stats.intellect + 10, GameConstants::MAX_STAT);
            stats.fatigue = std::min(stats.fatigue + 20, GameConstants::MAX_FATIGUE);
            stats.energy = std::max(stats.energy - 15, GameConstants::MIN_STAT);
            std::cout << "\nТы усердно готовился!\n";
            break;
        case 2:
            intellectBonus = 5;
            stats.intellect = std::min(stats.intellect + 5, GameConstants::MAX_STAT);
            stats.fatigue = std::min(stats.fatigue + 10, GameConstants::MAX_FATIGUE);
            std::cout << "\nТы повторил основные темы.\n";
            break;
        default:
            std::cout << "\nТы решил положиться на удачу.\n";
            break;
        }

        ConsoleUI::WaitForEnter();

        ConsoleUI::PrintHeader("РЕЗУЛЬТАТ ПЕРЕСДАЧИ");

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 100);
        int luck = dis(gen);

        int successChance = stats.intellect;
        if (intellectBonus > 0) successChance += 5;
        successChance = std::min(successChance, 95);

        bool success = (luck <= successChance);

        if (success) {
            std::cout << "\nТы успешно пересдал " << selectedExamName << "!\n";
            std::cout << "Преподаватель: «Вижу, вы подготовились. Зачтено.»\n";

            switch (selectedExamId) {
            case 1: historyGrade = 3; break;
            case 2: yampGrade = 3; break;
            case 4: discreteGrade = 3; break;
            case 5: calculusGrade = 3; break;
            }
            failedExamsCount--;

            stats.stress = std::max(stats.stress - 20, GameConstants::MIN_STAT);
            stats.humanity = std::min(stats.humanity + 10, GameConstants::MAX_STAT);
            std::cout << "-Стресс, +Человечность\n";
        }
        else {
            std::cout << "\nТы не смог пересдать " << selectedExamName << ".\n";
            std::cout << "Преподаватель: «Придёте в следующий раз.»\n";
            stats.stress = std::min(stats.stress + 20, GameConstants::MAX_STAT);
            std::cout << "+Стресс\n";
        }

        ConsoleUI::WaitForEnter();
    }
    else if (examChoice != 0) {
        std::cout << "\nНеверный выбор или экзамен уже сдан.\n";
        ConsoleUI::WaitForEnter();
    }
    else {
        std::cout << "\nТы решил не пересдавать экзамен.\n";
        ConsoleUI::WaitForEnter();
    }
}
// ========== ОБЫЧНЫЙ ДЕНЬ (без свидания и долгов) ==========
void Game::runNormalDay() {
    auto& stats = state.getPlayer().getStats();
    bool continueDay = true;
    bool hasDoneSomething = false;
    bool hasCalledMom = false;

    while (continueDay) {
        ConsoleUI::ClearScreen();
        ConsoleUI::PrintDayHeader(7, "Свободный день");
        ConsoleUI::PrintPlayerStats(state.getPlayer());

        std::cout << "\n------------------------------------------------------------\n";
        std::cout << "Чем хочешь заняться?\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "1. Встретиться с друзьями (настольные игры)\n";
        std::cout << "2. Позвонить маме\n";
        std::cout << "3. Почитать книгу дома\n";
        std::cout << "4. Остаться дома (сериал, ванна, ужин)\n";

        if (hasDoneSomething) {
            std::cout << "5. Закончить день\n";
        }
        else {
            std::cout << "5. (сначала сделай что-нибудь)\n";
        }
        std::cout << "------------------------------------------------------------\n";

        int choice;
        std::cin >> choice;
        std::cin.ignore(10000, '\n');

        switch (choice) {
        case 1: { // ВСТРЕЧА С ДРУЗЬЯМИ
            ConsoleUI::PrintHeader("ВСТРЕЧА С ДРУЗЬЯМИ");
            std::cout << "\nВы собираетесь у Семена\n";
            std::cout << "Они приходят с пиццей и колой.\n";
            ConsoleUI::WaitForEnter();

            ConsoleUI::PrintHeader("ВЫБОР ИГРЫ");
            std::cout << "\nБулат: «Ну что, во что играем?»\n";
            std::cout << "1. Монополия\n";
            std::cout << "2. Шахматы\n";
            std::cout << "3. Дурак\n";

            int gameChoice;
            std::cin >> gameChoice;
            std::cin.ignore(10000, '\n');

            if (gameChoice == 1) {
                ConsoleUI::PrintHeader("МОНОПОЛИЯ");
                std::cout << "\nВы играете в Монополию несколько часов.\n";
                std::cout << "Было весело и азартно!\n";
                state.getPlayer().modifyRelation("Булат", 5);
                state.getPlayer().modifyRelation("Семён", 5);
                stats.stress = std::max(stats.stress - 10, GameConstants::MIN_STAT);
                ConsoleUI::WaitForEnter();
            }
            else if (gameChoice == 2) {
                ConsoleUI::PrintHeader("ШАХМАТЫ");
                std::cout << "\nТы играешь в шахматы с Семёном.\n";
                std::cout << "Интеллектуальная битва!\n";

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, 100);
                int winChance = dis(gen);

                if (winChance <= stats.intellect) {
                    std::cout << "\nТы выиграл партию! Семён восхищён.\n";
                    state.getPlayer().modifyRelation("Семён", 8);
                    stats.intellect = std::min(stats.intellect + 2, GameConstants::MAX_STAT);
                }
                else {
                    std::cout << "\nТы проиграл. Семён дал несколько советов.\n";
                    state.getPlayer().modifyRelation("Семён", 3);
                    stats.intellect = std::min(stats.intellect + 1, GameConstants::MAX_STAT);
                }
                stats.stress = std::max(stats.stress - 10, GameConstants::MIN_STAT);
                ConsoleUI::WaitForEnter();
            }
            else if (gameChoice == 3) {
                ConsoleUI::PrintHeader("ДУРАК");
                std::cout << "\nБулат: «Сыграем в Дурака? Может, на интерес?»\n";
                std::cout << "Семён: «Я пас, просто посмотрю.»\n";
                std::cout << "Булат: «Ну что, сыграем на деньги?»\n\n";
                std::cout << "1. Да, сыграем на деньги (ставка 200 руб.)\n";
                std::cout << "2. Нет, сыграем просто так\n";

                int moneyGameChoice;
                std::cin >> moneyGameChoice;
                std::cin.ignore(10000, '\n');

                if (moneyGameChoice == 1 && stats.money >= 200) {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, 1);
                    int winChance = dis(gen);

                    if (winChance == 0) {
                        stats.money += 200;
                        std::cout << "\nТы выиграл 200 рублей!\n";
                        state.getPlayer().modifyRelation("Булат", 6);
                        stats.stress = std::max(stats.stress - 10, GameConstants::MIN_STAT);
                    }
                    else {
                        stats.money -= 200;
                        std::cout << "\nТы проиграл 200 рублей.\n";
                        state.getPlayer().modifyRelation("Булат", 3);
                        stats.stress = std::min(stats.stress + 10, GameConstants::MAX_STAT);
                    }
                }
                else {
                    std::cout << "\nВы играете в Дурака просто для удовольствия.\n";
                    state.getPlayer().modifyRelation("Булат", 4);
                    state.getPlayer().modifyRelation("Семён", 4);
                    stats.stress = std::max(stats.stress - 12, GameConstants::MIN_STAT);
                }
                ConsoleUI::WaitForEnter();
            }

            hasDoneSomething = true;
            continueDay = false;
            break;
        }

        case 2: { // ПОЗВОНИТЬ МАМЕ
            if (hasCalledMom) {
                std::cout << "\nТы уже звонил маме сегодня.\n";
                std::cout << "Не стоит отвлекать её ещё раз.\n";
                ConsoleUI::WaitForEnter();
                break;
            }

            ConsoleUI::PrintHeader("ЗВОНОК МАМЕ");
            std::cout << "\nТы набираешь номер мамы. Гудок... Гудок...\n";
            std::cout << "Мама: «Алло! Сынок! Как у тебя дела?»\n\n";

            int debts = state.getPlayer().getDebts();

            if (debts > 0) {
                std::cout << "Ты: «Привет, мам... Всё нормально, сессия закончилась.»\n";
                std::cout << "Мама: «Ты как-то неуверенно говоришь... Что-то случилось?»\n";
                std::cout << "Ты: «Да так... Не всё сдал. Есть небольшие проблемы.»\n";
                std::cout << "Мама: «Ох, сынок... Но ты не отчаивайся, в следующий раз получится.»\n";
                std::cout << "Ты: «Спасибо, мам. Я постараюсь.»\n\n";

                std::cout << "Разговор был грустным. Мама переживает за тебя.\n";
                state.getPlayer().modifyRelation("Мама", 2);
                stats.humanity = std::min(stats.humanity + 3, GameConstants::MAX_STAT);
                stats.stress = std::min(stats.stress + 5, GameConstants::MAX_STAT);
            }
            else {
                std::cout << "Ты: «Привет, мам! Всё отлично! Я сдал все экзамены!»\n";
                std::cout << "Мама: «Ой, какая радость! Я так за тебя переживала!»\n";
                std::cout << "Ты: «Да всё нормально, готовился, старался.»\n";
                std::cout << "Мама: «Я горжусь тобой, сынок! Ты у меня умница.»\n";
                std::cout << "Ты: «Спасибо, мам. Твоя поддержка очень помогает.»\n\n";

                std::cout << "Мама: «Слушай, я тут переведу тебе немного денег.\n";
                std::cout << "Купи себе что-нибудь вкусное, отдохни.»\n";
                std::cout << "Ты: «Мам, не надо, у меня всё есть...»\n";
                std::cout << "Мама: «Ничего не хочу слышать! Ты заслужил!»\n\n";

                stats.money += 300;
                state.getPlayer().modifyRelation("Мама", 10);
                stats.humanity = std::min(stats.humanity + 10, GameConstants::MAX_STAT);
                stats.stress = std::max(stats.stress - 15, GameConstants::MIN_STAT);
            }

            ConsoleUI::WaitForEnter();

            hasCalledMom = true;
            hasDoneSomething = true;
            break;
        }

        case 3: { // ПОЧИТАТЬ КНИГУ ДОМА
            ConsoleUI::PrintHeader("ВЫБОР КНИГИ");
            std::cout << "\nТы устроился в кресле с книгой.\n";
            std::cout << "Какую книгу выберешь?\n";
            std::cout << "1. «Искусство программирования» (+8 интеллекта)\n";
            std::cout << "2. «Великий Гэтсби» (+5 романтики)\n";
            std::cout << "3. «Как перестать беспокоиться» (-15 стресса)\n";
            std::cout << "4. Сборник смешных историй (-10 стресса)\n";

            int bookChoice;
            std::cin >> bookChoice;
            std::cin.ignore(10000, '\n');

            switch (bookChoice) {
            case 1:
                stats.intellect = std::min(stats.intellect + 8, GameConstants::MAX_STAT);
                std::cout << "\nТы погрузился в мир алгоритмов и кода.\n";
                break;
            case 2:
                stats.romance = std::min(stats.romance + 5, GameConstants::MAX_STAT);
                std::cout << "\nИстория любви тронула твоё сердце.\n";
                break;
            case 3:
                stats.stress = std::max(stats.stress - 15, GameConstants::MIN_STAT);
                std::cout << "\nТы узнал несколько техник дыхания.\n";
                break;
            case 4:
                stats.stress = std::max(stats.stress - 10, GameConstants::MIN_STAT);
                std::cout << "\nТы хохотал до слёз!\n";
                break;
            default:
                std::cout << "\nТы просто пролистал книгу без интереса.\n";
                break;
            }

            ConsoleUI::WaitForEnter();

            hasDoneSomething = true;
            continueDay = false;
            break;
        }

        case 4: { // ОСТАТЬСЯ ДОМА
            ConsoleUI::PrintHeader("ДОМАШНИЙ ВЕЧЕР");
            std::cout << "\nТы решил остаться дома и устроить себе спокойный вечер.\n";
            ConsoleUI::WaitForEnter();

            ConsoleUI::PrintHeader("ПРОСМОТР СЕРИАЛА");
            std::cout << "\nТы включил новый сериал, который давно хотел посмотреть.\n";
            stats.stress = std::max(stats.stress - 15, GameConstants::MIN_STAT);
            stats.energy = std::max(stats.energy - 5, GameConstants::MIN_STAT);
            ConsoleUI::WaitForEnter();

            ConsoleUI::PrintHeader("РАССЛАБЛЯЮЩАЯ ВАННА");
            std::cout << "\nТы решил принять тёплую ванну.\n";
            stats.energy = std::min(stats.energy + 20, GameConstants::MAX_STAT);
            stats.stress = std::max(stats.stress - 10, GameConstants::MIN_STAT);
            ConsoleUI::WaitForEnter();

            ConsoleUI::PrintHeader("УЖИН");
            std::cout << "\nТы приготовил себе ужин.\n";
            stats.hunger = std::max(stats.hunger - 25, GameConstants::MIN_STAT);
            ConsoleUI::WaitForEnter();

            ConsoleUI::PrintHeader("ПОДГОТОВКА КО СНУ");
            std::cout << "\nТы почистил зубы и забрался в кровать.\n";
            stats.energy = std::min(stats.energy + 10, GameConstants::MAX_STAT);
            stats.fatigue = std::max(stats.fatigue - 15, GameConstants::MIN_STAT);
            ConsoleUI::WaitForEnter();

            hasDoneSomething = true;
            continueDay = false;
            break;
        }

        case 5:
            if (hasDoneSomething) {
                continueDay = false;
            }
            else {
                std::cout << "\nТы только проснулся. Сделай хотя бы что-нибудь!\n";
                ConsoleUI::WaitForEnter();
            }
            break;

        default:
            std::cout << "\nНеверный выбор.\n";
            ConsoleUI::WaitForEnter();
            break;
        }
    }
}
// ==================== ДЕНЬ 8 — ИТОГИ ====================

void Game::runDay8() {
    ConsoleUI::PrintDayHeader(8, "Итоги месяца");
    std::cout << "\nДеканат публикует результаты сессии.\n";
    std::cout << "Ты подходишь к стенду с объявлениями...\n";
    ConsoleUI::WaitForEnter();

    auto& stats = state.getPlayer().getStats();
    int allaRelation = state.getPlayer().getRelation("Алла");

    // Подсчитываем результаты
    int examsPassed = 0;      // Всего сдано (оценка >= 3)
    int excellentCount = 0;   // Сдано на 4 и 5
    int perfectCount = 0;     // Сдано на 5

    if (historyGrade >= 3) examsPassed++;
    if (historyGrade >= 4) excellentCount++;
    if (historyGrade == 5) perfectCount++;

    if (yampGrade >= 3) examsPassed++;
    if (yampGrade >= 4) excellentCount++;
    if (yampGrade == 5) perfectCount++;

    if (discreteGrade >= 3) examsPassed++;
    if (discreteGrade >= 4) excellentCount++;
    if (discreteGrade == 5) perfectCount++;

    if (calculusGrade >= 3) examsPassed++;
    if (calculusGrade >= 4) excellentCount++;
    if (calculusGrade == 5) perfectCount++;

    // ========== РАССКАЗЧИК (итоги) ==========
    ConsoleUI::PrintHeader("ДЕКАНАТ СООБЩАЕТ");

    std::cout << "«Сессия завершена. Вот твои результаты:\n\n";

    std::cout << "- История: ";
    if (historyGrade == 2) std::cout << "не сдана (2)";
    else if (historyGrade >= 3) std::cout << "сдана на " << historyGrade;
    else std::cout << "не сдавался";
    std::cout << "\n";

    std::cout << "- Языки и методы программирования: ";
    if (yampGrade == 2) std::cout << "не сдан (2)";
    else if (yampGrade >= 3) std::cout << "сдан на " << yampGrade;
    else std::cout << "не сдавался";
    std::cout << "\n";

    std::cout << "- Дискретная математика: ";
    if (discreteGrade == 2) std::cout << "не сдана (2)";
    else if (discreteGrade >= 3) std::cout << "сдана на " << discreteGrade;
    else std::cout << "не сдавался";
    std::cout << "\n";

    std::cout << "- Математический анализ: ";
    if (calculusGrade == 2) std::cout << "не сдан (2)";
    else if (calculusGrade >= 3) std::cout << "сдан на " << calculusGrade;
    else std::cout << "не сдавался";
    std::cout << "\n";

    std::cout << "\n- Количество долгов: " << failedExamsCount << "\n";
    std::cout << "- Интеллект: " << stats.intellect << "/100\n";
    std::cout << "- Деньги: " << stats.money << " руб.\n";
    std::cout << "- Энергия: " << stats.energy << "/100\n";
    std::cout << "- Отношения с Аллой: " << allaRelation << "/100\n";

    int avgFriends = (state.getPlayer().getRelation("Булат") + state.getPlayer().getRelation("Семён")) / 2;
    std::cout << "- Отношения с друзьями: " << avgFriends << "/100\n";

    std::cout << "\n»\n";
    std::cout << "------------------------------------------------------------\n";
    ConsoleUI::WaitForEnter();

    // ========== УРОК ==========
    ConsoleUI::PrintHeader("РАЗМЫШЛЕНИЯ");

    if (failedExamsCount == 0 && excellentCount >= 3) {
        std::cout << "\nТы думаешь: «Сессия прошла отлично! Я справился со всеми испытаниями.»\n";
        std::cout << "Ты понимаешь, что упорство и труд действительно приносят результат.\n";
    }
    else if (failedExamsCount == 0) {
        std::cout << "\nТы думаешь: «Неплохо. Всё сдал, но могло быть и лучше.»\n";
        std::cout << "Ты понимаешь, что нужно больше внимания уделять учёбе.\n";
    }
    else if (failedExamsCount <= 2) {
        std::cout << "\nТы думаешь: «Есть долги, но это не катастрофа.»\n";
        std::cout << "Ты понимаешь, что в следующем семестре нужно браться за ум.\n";
    }
    else {
        std::cout << "\nТы думаешь: «Это провал... Как я мог так запустить учёбу?»\n";
        std::cout << "Ты понимаешь, что последствия могут быть серьёзными.\n";
    }

    ConsoleUI::WaitForEnter();

    // ========== КОНЦОВКИ ==========
    ConsoleUI::PrintHeader("ИТОГ");

    // Секретная концовка
    if (perfectCount == 4 && allaRelation >= 90) {
        std::cout << "\n************************************************************\n";
        std::cout << "Алла подходит к тебе, её глаза сияют:\n";
        std::cout << "«Ты знаешь... Я всё это время хотела тебе сказать...\n";
        std::cout << "Ты мне очень нравишься. С первой нашей встречи.»\n";
        std::cout << "Она краснеет и берёт тебя за руку.\n";
        std::cout << "Ты чувствуешь, что это начало чего-то прекрасного.\n\n";
        std::cout << "************************************************************\n";
        std::cout << "\nПОЗДРАВЛЯЕМ! ВЫ ПОЛУЧИЛИ ЛУЧШУЮ КОНЦОВКУ!\n";
        state.getPlayer().setFlag("secret_ending", true);
    }
    // Концовка "Стипендия" (с Аллой)
    else if (failedExamsCount == 0 && excellentCount >= 3 && allaRelation >= 50) {
        std::cout << "\nТвой телефон пиликает. Сообщение от Аллы:\n";
        std::cout << "«Ну что, отличник, пошли отмечать? Жду в парке!»\n\n";
        std::cout << "Ты улыбаешься и собираешься на встречу.\n";
        std::cout << "Сессия позади, а впереди — заслуженный отдых.\n";
    }
    // Концовка "Стипендия без Аллы" (отличная учёба, но с Аллой не сложилось)
    else if (failedExamsCount == 0 && excellentCount >= 3 && allaRelation < 50) {
        std::cout << "\nТы получил стипендию! Декан хвалит тебя на собрании.\n";
        std::cout << "Но в душе ты чувствуешь пустоту... С Аллой у вас так и не сложилось.\n";
        std::cout << "Ты решаешь посвятить себя учёбе и карьере.\n";
    }
    // Концовка "Обычный студент" (всё сдано, но без стипендии)
    else if (failedExamsCount == 0) {
        std::cout << "\nТы сдал все экзамены, но без стипендии.\n";
        std::cout << "Нормальная студенческая жизнь продолжается.\n";
        std::cout << "В следующем семестре нужно постараться больше.\n";
    }
    // Концовка "Вечный должник" (1-2 долга)
    else if (failedExamsCount <= 2 && failedExamsCount > 0) {
        std::cout << "\nУ тебя есть долги, но ты продолжаешь учиться.\n";
        std::cout << "В следующем семестре придётся пересдавать.\n";
        std::cout << "Главное — не бросить всё на полпути.\n";
    }
    // Концовка "Академ" (3 долга)
    else if (failedExamsCount == 3) {
        std::cout << "\nДеканат: «У вас слишком много академических задолженностей.\n";
        std::cout << "Мы вынуждены отправить вас в академический отпуск.»\n\n";
        std::cout << "Ты берёшь паузу в учёбе, чтобы подумать о будущем.\n";
    }
    // Концовка "Отчисление" (4 и более долгов)
    else if (failedExamsCount >= 4) {
        std::cout << "\nДеканат: «Студент Тимур, вы отчислены за неуспеваемость.\n";
        std::cout << "Заберите документы в деканате.»\n\n";
        std::cout << "Ты стоишь с папкой в руках и понимаешь:\n";
        std::cout << "Надо было больше учиться...\n";
    }

    ConsoleUI::WaitForEnter();
    ConsoleUI::PrintHeader("КОНЕЦ");
    std::cout << "\nСпасибо за игру!\n";
    ConsoleUI::WaitForEnter();

}