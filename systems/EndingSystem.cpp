#include "EndingSystem.h"
#include <iostream>

GameOverCondition EndingSystem::CheckEnding(const Player& player) {
    // Проверяем смертельные условия
    if (player.getStats().health <= 0) {
        return GameOverCondition::StarvedToDeath;
    }

    const auto& stats = player.getStats();

    // Смерть от голода
    if (stats.hunger >= 100) {
        return GameOverCondition::StarvedToDeath;
    }

    // Психбольница
    if (stats.stress >= 100) {
        return GameOverCondition::MentalHospital;
    }

    return GameOverCondition::None;
}

GameOverCondition EndingSystem::EvaluateEnding(const Player& player) {
    const auto& stats = player.getStats();
    int avgGrade = 0;
    int gradeCount = 0;
    for (int i = 1; i <= 5; i++) {
        if (player.getGrade(i) > 0) {
            avgGrade += player.getGrade(i);
            gradeCount++;
        }
    }
    if (gradeCount > 0) avgGrade /= gradeCount;

    // Проверка особых концовок

    // Секретная романтическая
    if (player.hasFlag("romantic_ending") &&
        stats.romance >= 70 &&
        player.getRelation("Алла") >= 80 &&
        avgGrade >= 60) {
        return GameOverCondition::SecretRomantic;
    }

    // Секретная армейская
    if (player.hasFlag("army_path")) {
        return GameOverCondition::SecretArmy;
    }

    // Вечный должник
    if (player.getDebts() >= 4) {
        return GameOverCondition::EternalDebtor;
    }

    // Отчисление
    if (player.getDebts() >= 2 && avgGrade < 50) {
        return GameOverCondition::Expelled;
    }

    // Академ
    if (stats.stress >= 80 && stats.fatigue >= 80) {
        return GameOverCondition::AcademicLeave;
    }

    // Оценка итогов
    int totalScore = 0;

    totalScore += stats.intellect / 2;
    totalScore += stats.humanity;
    totalScore += stats.romance > 50 ? 20 : 0;
    totalScore += avgGrade;
    totalScore -= stats.stress;
    totalScore -= stats.fatigue / 2;
    totalScore += player.getRelation("Преподаватели");

    if (totalScore >= 300) return GameOverCondition::SuperEnding;
    if (totalScore >= 200) return GameOverCondition::GoodEnding;
    return GameOverCondition::NormalEnding;
}

std::string EndingSystem::GetEndingText(GameOverCondition ending) {
    switch (ending) {
    case GameOverCondition::SuperEnding:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                     СУПЕР-КОНЦОВКА                          ║
║                                                            ║
║ Тимур сдал все экзамены на отлично!                         ║
║ Преподаватели восхищены его знаниями.                        ║
║ Друзья гордятся им.                                         ║
║ А Алла... У них с Тимуром настоящая любовь.                 ║
║                                                            ║
║ Тимур получил красный диплом и предложение                  ║
║ о работе в лучшей IT-компании страны.                       ║
║                                                            ║
║ БУДНИ СТУДЕНТА ЗАКОНЧИЛИСЬ. НАЧИНАЕТСЯ ЖИЗНЬ!              ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::GoodEnding:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                     ХОРОШАЯ КОНЦОВКА                        ║
║                                                            ║
║ Тимур успешно сдал сессию.                                  ║
║ Не всё было гладко, но он справился.                        ║
║ Впереди лето, каникулы и новые планы.                       ║
║                                                            ║
║ Жизнь продолжается!                                        ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::NormalEnding:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                    НОРМАЛЬНАЯ КОНЦОВКА                      ║
║                                                            ║
║ Сессия позади. Было всякое.                                 ║
║ Тимур выжил и это главное.                                  ║
║ Впереди ещё много семестров...                              ║
║                                                            ║
║ В целом, всё нормально.                                    ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::Expelled:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                      ОТЧИСЛЕНИЕ                             ║
║                                                            ║
║ Тимура отчислили за неуспеваемость.                         ║
║ Слишком много долгов, слишком мало знаний.                  ║
║                                                            ║
║ Родители расстроены. Будущее туманно.                       ║
║ Но это не конец света...                                    ║
║ Хотя сейчас кажется, что да.                                ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::AcademicLeave:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                       АКАДЕМ                                ║
║                                                            ║
║ Тимур ушёл в академический отпуск.                          ║
║ Нервы сдали, сил больше нет.                                ║
║                                                            ║
║ Возможно, через год он вернётся.                            ║
║ А может быть, найдёт другой путь.                           ║
║                                                            ║
║ Главное — отдохнуть и прийти в себя.                       ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::MentalHospital:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                     ПСИХБОЛЬНИЦА                            ║
║                                                            ║
║ Стресс сломал Тимура.                                       ║
║ Бессонные ночи, постоянное напряжение...                     ║
║                                                            ║
║ Теперь ему нужна помощь специалистов.                       ║
║ Хорошая новость: здесь не надо сдавать экзамены.            ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::StarvedToDeath:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                   СМЕРТЬ ОТ ГОЛОДА                          ║
║                                                            ║
║ Тимур забывал есть так часто, что организм не выдержал.     ║
║                                                            ║
║ Экзамены уже не важны.                                      ║
║ Важно было вовремя пообедать...                             ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::EternalDebtor:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                    ВЕЧНЫЙ ДОЛЖНИК                           ║
║                                                            ║
║ Тимур накопил столько долгов, что их не покрыть.            ║
║ Сессия не сдана, хвосты растут.                             ║
║                                                            ║
║ Придётся брать ещё один семестр...                          ║
║ И ещё один...                                               ║
║ И ещё...                                                    ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::SecretRomantic:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║              СЕКРЕТНАЯ РОМАНТИЧЕСКАЯ КОНЦОВКА              ║
║                                                            ║
║ Тимур и Алла — идеальная пара.                              ║
║                                                            ║
║ Вместе они сдали все экзамены.                              ║
║ Вместе строят планы на будущее.                             ║
║                                                            ║
║ «И жили они долго и счастливо...»                           ║
║                                                            ║
║ Ну, по крайней мере, до следующей сессии.                  ║
╚══════════════════════════════════════════════════════════════╝
)";

    case GameOverCondition::SecretArmy:
        return R"(
╔══════════════════════════════════════════════════════════════╗
║                СЕКРЕТНАЯ АРМЕЙСКАЯ КОНЦОВКА                 ║
║                                                            ║
║ Тимур решил, что учёба — не его путь.                       ║
║ Армия ждёт!                                                 ║
║                                                            ║
║ Два года срочной службы.                                    ║
║ Новые друзья, новые испытания.                              ║
║                                                            ║
║ Может быть, вернётся доучиваться.                           ║
║ А может быть, выберет другую дорогу.                        ║
╚══════════════════════════════════════════════════════════════╝
)";

    default:
        return "Неизвестная концовка.";
    }
}
