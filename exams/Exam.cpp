#include "Exam.h"
#include "../ui/ConsoleUI.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <cctype>
#include <sstream>

namespace {
    std::string normalizeAnswer(const std::string& text) {
        std::string result;
        bool lastWasSpace = false;

        for (unsigned char ch : text) {
            if (std::isspace(ch)) {
                if (!lastWasSpace && !result.empty()) {
                    result += ' ';
                    lastWasSpace = true;
                }
            } else {
                result += static_cast<char>(std::tolower(ch));
                lastWasSpace = false;
            }
        }

        while (!result.empty() && result.back() == ' ') {
            result.pop_back();
        }

        return result;
    }

    bool isTextAnswerCorrect(const std::string& playerAnswer, const std::string& correctAnswer) {
        std::string normalizedPlayer = normalizeAnswer(playerAnswer);

        if (normalizedPlayer.empty()) {
            return false;
        }

        // Несколько правильных вариантов можно писать через |
        // Например: "vector|std::vector|вектор"
        std::stringstream ss(correctAnswer);
        std::string variant;

        while (std::getline(ss, variant, '|')) {
            std::string normalizedVariant = normalizeAnswer(variant);

            if (normalizedPlayer == normalizedVariant) {
                return true;
            }
        }

        return false;
    }
}

int Exam::evaluateAnswers(const std::vector<int>& answers) const {
    if (answers.empty() || questions.empty()) {
        return 0;
    }

    int correct = 0;

    for (size_t i = 0; i < std::min(answers.size(), questions.size()); i++) {
        if (answers[i] == 1) {
            correct++;
        }
    }

    return (correct * 100) / static_cast<int>(questions.size());
}

void Exam::displayQuestions() const {
    ConsoleUI::PrintHeader("ЭКЗАМЕН: " + name);
    std::cout << "Преподаватель: " << teacherName << "\n";
    std::cout << "Количество вопросов: " << questions.size() << "\n\n";
}

int Exam::askQuestionsConsole(Player& player) const {
    int correctCount = 0;
    int totalQuestions = static_cast<int>(questions.size());

    if (totalQuestions <= 0) {
        return 0;
    }

    for (int i = 0; i < totalQuestions; i++) {
        const auto& q = questions[i];

        ConsoleUI::PrintSeparator();

        std::cout << "Вопрос " << (i + 1) << "/" << totalQuestions << ":\n";
        std::cout << q.question << "\n\n";

        if (q.isMultipleChoice && !q.options.empty()) {
            for (size_t j = 0; j < q.options.size(); j++) {
                std::cout << (j + 1) << ". " << q.options[j] << "\n";
            }

            std::cout << "\nВведите номер ответа: ";

            int choice = ConsoleUI::ReadInt("", 1, static_cast<int>(q.options.size()));

            if (q.options[choice - 1] == q.correctAnswer) {
                std::cout << "✓ Правильно!\n";
                correctCount++;
            } else {
                std::cout << "✗ Неправильно. Правильный ответ: " << q.correctAnswer << "\n";
            }
        } else {
            std::cout << "Введите ваш ответ: ";

            std::string answer;
            std::getline(std::cin, answer);

            if (isTextAnswerCorrect(answer, q.correctAnswer)) {
                std::cout << "✓ Правильно!\n";
                correctCount++;
            } else {
                std::cout << "✗ Неправильно. Правильный ответ: " << q.correctAnswer << "\n";
            }
        }
    }

    ConsoleUI::PrintSeparator();

    int baseScore = (correctCount * 100) / totalQuestions;

    int effectiveIntellect = player.getStats().intellect;

    if (player.hasBuff(BuffType::ImposterSyndrome)) {
        effectiveIntellect = effectiveIntellect * 80 / 100;
    }

    int intellectBonus = 0;

    if (effectiveIntellect >= 90) {
        intellectBonus = 25;
    } else if (effectiveIntellect >= 75) {
        intellectBonus = 20;
    } else if (effectiveIntellect >= 60) {
        intellectBonus = 12;
    } else if (effectiveIntellect >= 45) {
        intellectBonus = 5;
    } else if (effectiveIntellect >= 30) {
        intellectBonus = 0;
    } else {
        intellectBonus = -10;
    }

    // difficulty:
    // 40 = лёгкий экзамен
    // 50 = обычный экзамен
    // 70 = тяжёлый экзамен
    int difficultyModifier = (50 - difficulty) / 2;

    int score = std::clamp(
        baseScore + intellectBonus + difficultyModifier,
        0,
        100
    );

    std::cout << "Результат: " << correctCount << " из " << totalQuestions << " правильных.\n";
    std::cout << "Базовый балл за ответы: " << baseScore << "\n";

    if (player.hasBuff(BuffType::ImposterSyndrome)) {
        std::cout << "Синдром самозванца мешает сосредоточиться: эффективный интеллект снижен.\n";
    }

    std::cout << "Бонус интеллекта: "
              << (intellectBonus >= 0 ? "+" : "") << intellectBonus << "\n";

    std::cout << "Модификатор сложности экзамена: "
              << (difficultyModifier >= 0 ? "+" : "") << difficultyModifier << "\n";

    std::cout << "Итог до реакции преподавателя: " << score << "\n";

    return score;
}

std::string Exam::getTeacherReaction(int score) const {
    if (score >= GameConstants::EXAM_EXCELLENT_THRESHOLD) {
        return teacherName + ": «Отлично! Вы прекрасно знаете предмет!»";
    } else if (score >= GameConstants::EXAM_PASS_THRESHOLD) {
        return teacherName + ": «Неплохо, но могло быть и лучше.»";
    } else {
        return teacherName + ": «Это провал. Придётся пересдавать.»";
    }
}

// ---- HistoryExam ----

HistoryExam::HistoryExam() {
    name = "История";
    teacherName = "Елена Викторовна";
    difficulty = 40;
}

void HistoryExam::generateQuestions() {
    questions.clear();

    questions.push_back({
        "В каком году была Куликовская битва?",
        "1380",
        {"1242", "1380", "1480", "1612"},
        true
    });

    questions.push_back({
        "Кто был первым президентом России?",
        "Борис Ельцин",
        {"Михаил Горбачёв", "Борис Ельцин", "Владимир Путин", "Дмитрий Медведев"},
        true
    });

    questions.push_back({
        "Какое событие произошло в 1917 году?",
        "Октябрьская революция",
        {"Первая мировая война", "Октябрьская революция", "Отмена крепостного права", "Война 1812 года"},
        true
    });

    questions.push_back({
        "Кто написал 'Слово о полку Игореве'?",
        "Неизвестный автор",
        {"Пушкин", "Лермонтов", "Неизвестный автор", "Толстой"},
        true
    });

    questions.push_back({
        "В каком году была основана Москва?",
        "1147",
        {"1147", "1237", "1328", "1480"},
        true
    });
}

int HistoryExam::runExam(Player& player) {
    generateQuestions();
    displayQuestions();

    std::cout << "\nВыберите билет:\n";
    std::cout << "1. Левый билет\n";
    std::cout << "2. Средний билет\n";
    std::cout << "3. Правый билет\n";
    std::cout << "Ваш выбор: ";

    int ticketChoice = ConsoleUI::ReadInt("", 1, 3);

    int luckBonus = 0;

    if (ticketChoice == 2) {
        luckBonus = 10;
        std::cout << "\nСредний билет оказался удачным!\n";
    } else if (ticketChoice == 1 || ticketChoice == 3) {
        luckBonus = -5;
        std::cout << "\nБилет попался сложный.\n";
    }

    int score = askQuestionsConsole(player);

    score = std::clamp(
        score + luckBonus + (player.getRelation("Преподаватели") / 10),
        0,
        100
    );

    std::cout << "\n" << getTeacherReaction(score) << "\n";

    if (score >= GameConstants::EXAM_PASS_THRESHOLD) {
        player.setGrade(1, score);
        player.removeDebt();
        std::cout << "Экзамен сдан!\n";
    } else {
        player.addDebt();
        std::cout << "Экзамен провален. Появился долг.\n";
    }

    return score;
}

// ---- YAMPExam ----

YAMPExam::YAMPExam() {
    name = "Язык и мат. программирования";
    teacherName = "Михаил Олегович";
    difficulty = 55;
}

void YAMPExam::generateQuestions() {
    questions.clear();

    questions.push_back({
        "Что такое указатель в C++?",
        "Переменная для хранения адреса памяти",
        {"Тип данных", "Переменная для хранения адреса памяти", "Функция", "Класс"},
        true
    });

    questions.push_back({
        "Какой оператор используется для выделения памяти в C++?",
        "new",
        {"malloc", "new", "alloc", "create"},
        true
    });

    questions.push_back({
        "Что такое виртуальная функция?",
        "Функция, которая может быть переопределена в наследнике",
        {
            "Статическая функция",
            "Функция, которая может быть переопределена в наследнике",
            "Дружественная функция",
            "Встроенная функция"
        },
        true
    });

    questions.push_back({
        "Какой контейнер реализует динамический массив в STL?",
        "vector",
        {"list", "vector", "map", "set"},
        true
    });

    questions.push_back({
        "Что делает delete[]?",
        "Освобождает массив в памяти",
        {
            "Освобождает одну переменную",
            "Освобождает массив в памяти",
            "Удаляет файл",
            "Вызывает деструктор"
        },
        true
    });
}

int YAMPExam::runExam(Player& player) {
    generateQuestions();
    displayQuestions();

    int score = askQuestionsConsole(player);

    score = std::clamp(
        score + (player.getRelation("Преподаватели") / 10),
        0,
        100
    );

    std::cout << "\n" << getTeacherReaction(score) << "\n";

    if (score >= GameConstants::EXAM_PASS_THRESHOLD) {
        player.setGrade(2, score);
        player.removeDebt();
        std::cout << "Экзамен сдан!\n";
    } else {
        player.addDebt();
        std::cout << "Экзамен провален. Появился долг.\n";
    }

    return score;
}

// ---- DiscreteExam ----

DiscreteExam::DiscreteExam() {
    name = "Дискретная математика";
    teacherName = "Наталья Петровна";
    difficulty = 65;
}

void DiscreteExam::generateQuestions() {
    questions.clear();

    questions.push_back({
        "Что такое граф?",
        "Множество вершин и рёбер",
        {"Множество чисел", "Множество вершин и рёбер", "Матрица", "Функция"},
        true
    });

    questions.push_back({
        "Сколько существует булевых функций от 2 переменных?",
        "16",
        {"4", "8", "16", "2"},
        true
    });

    questions.push_back({
        "Что такое отношение эквивалентности?",
        "Рефлексивное, симметричное, транзитивное отношение",
        {
            "Симметричное отношение",
            "Рефлексивное, симметричное, транзитивное отношение",
            "Транзитивное отношение",
            "Рефлексивное отношение"
        },
        true
    });

    questions.push_back({
        "Чему равно 2^5 mod 7?",
        "4",
        {"2", "3", "4", "5"},
        true
    });

    questions.push_back({
        "Что такое деревья в теории графов?",
        "Связные ациклические графы",
        {"Полные графы", "Ориентированные графы", "Связные ациклические графы", "Пустые графы"},
        true
    });
}

int DiscreteExam::runExam(Player& player) {
    generateQuestions();
    displayQuestions();

    int score = askQuestionsConsole(player);

    score = std::clamp(
        score + (player.getRelation("Преподаватели") / 10),
        0,
        100
    );

    std::cout << "\n" << getTeacherReaction(score) << "\n";

    if (score >= GameConstants::EXAM_PASS_THRESHOLD) {
        player.setGrade(3, score);
        player.removeDebt();
        std::cout << "Экзамен сдан!\n";
    } else {
        player.addDebt();
        std::cout << "Экзамен провален. Появился долг.\n";
    }

    return score;
}

// ---- CalculusExam ----

CalculusExam::CalculusExam() {
    name = "Математический анализ";
    teacherName = "Александр Сергеевич";
    difficulty = 70;
}

void CalculusExam::generateQuestions() {
    questions.clear();

    questions.push_back({
        "Чему равна производная sin(x)?",
        "cos(x)",
        {"cos(x)", "-sin(x)", "tg(x)", "cot(x)"},
        true
    });

    questions.push_back({
        "Что такое определённый интеграл?",
        "Площадь под кривой",
        {"Производная", "Площадь под кривой", "Предел функции", "Ряд"},
        true
    });

    questions.push_back({
        "Какая формула для производной произведения?",
        "(uv)' = u'v + uv'",
        {"(uv)' = u'v'", "(uv)' = u'v + uv'", "(uv)' = u'v - uv'", "(uv)' = u + v"},
        true
    });

    questions.push_back({
        "Чему равен предел sin(x)/x при x→0?",
        "1",
        {"0", "1", "∞", "-1"},
        true
    });

    questions.push_back({
        "Что такое ряд Тейлора?",
        "Разложение функции в степенной ряд",
        {"Сумма чисел", "Разложение функции в степенной ряд", "Производная высшего порядка", "Интеграл"},
        true
    });
}

int CalculusExam::runExam(Player& player) {
    generateQuestions();
    displayQuestions();

    int score = askQuestionsConsole(player);

    score = std::clamp(
        score + (player.getRelation("Преподаватели") / 10),
        0,
        100
    );

    std::cout << "\n" << getTeacherReaction(score) << "\n";

    if (score >= GameConstants::EXAM_PASS_THRESHOLD) {
        player.setGrade(4, score);
        player.removeDebt();
        std::cout << "Экзамен сдан!\n";
    } else {
        player.addDebt();
        std::cout << "Экзамен провален. Появился долг.\n";
    }

    return score;
}

// ---- NetworksExam ----

NetworksExam::NetworksExam() {
    name = "Компьютерные сети";
    teacherName = "Денис Игоревич";
    difficulty = 50;
}

void NetworksExam::generateQuestions() {
    questions.clear();

    questions.push_back({
        "Какая модель описывает взаимодействие сетевых протоколов?",
        "OSI",
        {"TCP/IP", "OSI", "HTTP", "FTP"},
        true
    });

    questions.push_back({
        "Что такое IP-адрес?",
        "Уникальный идентификатор устройства в сети",
        {"Номер порта", "Уникальный идентификатор устройства в сети", "Доменное имя", "MAC-адрес"},
        true
    });

    questions.push_back({
        "Какой протокол используется для передачи веб-страниц?",
        "HTTP",
        {"FTP", "HTTP", "SMTP", "DNS"},
        true
    });

    questions.push_back({
        "Что такое маска подсети?",
        "Определяет сетевую и хостовую часть адреса",
        {"Пароль", "Определяет сетевую и хостовую часть адреса", "Шифрование", "Маршрут"},
        true
    });

    questions.push_back({
        "Сколько бит в IPv4-адресе?",
        "32",
        {"16", "32", "64", "128"},
        true
    });
}

int NetworksExam::runExam(Player& player) {
    generateQuestions();
    displayQuestions();

    int score = askQuestionsConsole(player);

    score = std::clamp(
        score + (player.getRelation("Преподаватели") / 10),
        0,
        100
    );

    std::cout << "\n" << getTeacherReaction(score) << "\n";

    if (score >= GameConstants::EXAM_PASS_THRESHOLD) {
        player.setGrade(5, score);
        player.removeDebt();
        std::cout << "Экзамен сдан!\n";
    } else {
        player.addDebt();
        std::cout << "Экзамен провален. Появился долг.\n";
    }

    return score;
}