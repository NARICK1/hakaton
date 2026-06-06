#include "RandomEvent.h"
#include "../ui/ConsoleUI.h"
#include <random>
#include <iostream>
#include <algorithm>

RandomEventManager::RandomEventManager() {
    initEvents();
}

void RandomEventManager::initEvents() {
    events = {
        {
            "Находка",
            "Ты нашёл на улице 200 рублей!",
            [](Player& p) { p.getStats().money += 200; std::cout << "+200 рублей!\n"; },
            15, 1, 8
        },
        {
            "Потеря",
            "Ты потерял 150 рублей из кармана.",
            [](Player& p) { p.getStats().money = std::max(0, p.getStats().money - 150); std::cout << "-150 рублей...\n"; },
            10, 1, 8
        },
        {
            "Встреча знакомого",
            "На улице ты встретил старого друга. Он подбодрил тебя.",
            [](Player& p) { p.getStats().stress = std::max(0, p.getStats().stress - 10); p.getStats().energy += 5; std::cout << "Стресс -10, энергия +5\n"; },
            12, 1, 8
        },
        {
            "Хорошая еда",
            "В столовой сегодня дают бесплатные обеды!",
            [](Player& p) { p.getStats().hunger = std::max(0, p.getStats().hunger - 30); p.getStats().energy += 10; std::cout << "Голод -30, энергия +10\n"; },
            8, 1, 8
        },
        {
            "Плохая еда",
            "Ты съел что-то не то в столовой. Теперь плохо.",
            [](Player& p) { p.getStats().hunger += 10; p.getStats().energy -= 10; p.getStats().health -= 5; std::cout << "Голод +10, энергия -10, здоровье -5\n"; },
            8, 1, 8
        },
        {
            "Помощь преподавателя",
            "Преподаватель заметил твои старания и предложил дополнительное занятие.",
            [](Player& p) { p.getStats().intellect += 5; p.modifyRelation("Преподаватели", 5); std::cout << "Интеллект +5, отношения с преподавателями +5\n"; },
            10, 1, 8
        },
        {
            "Анекдот на экзамене",
            "Ты рассказал анекдот, и преподаватель посмеялся. Настроение улучшилось!",
            [](Player& p) { p.getStats().stress = std::max(0, p.getStats().stress - 15); p.modifyRelation("Преподаватели", 3); std::cout << "Стресс -15, отношения с преподавателями +3\n"; },
            7, 1, 8
        },
        {
            "Сосед-музыкант",
            "Сосед всю ночь играл на гитаре. Ты не выспался.",
            [](Player& p) { p.getStats().energy -= 15; p.getStats().fatigue += 15; p.getStats().stress += 5; std::cout << "Энергия -15, усталость +15, стресс +5\n"; },
            10, 1, 8
        },
        {
            "Удачная покупка",
            "Ты купил товары со скидкой и сэкономил 200 рублей!",
            [](Player& p) { p.getStats().money += 200; std::cout << "+200 рублей!\n"; },
            8, 1, 8
        },
        {
            "Звонок родителей",
            "Родители позвонили и подбодрили тебя. Ты чувствуешь прилив сил.",
            [](Player& p) { p.getStats().energy += 10; p.getStats().stress = std::max(0, p.getStats().stress - 10); std::cout << "Энергия +10, стресс -10\n"; },
            12, 1, 8
        }
    };
}

bool RandomEventManager::tryTriggerEvent(Player& player, int day) {
    if (events.empty()) return false;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> chance(0, 100);

    // 30% шанс на событие
    if (chance(rng) > 30) return false;

    // Фильтруем доступные события
    std::vector<RandomEvent*> available;
    for (auto& e : events) {
        if (day >= e.minDay && day <= e.maxDay) {
            available.push_back(&e);
        }
    }

    if (available.empty()) return false;

    // Выбираем случайное событие с учётом веса
    int totalWeight = 0;
    for (auto* e : available) totalWeight += e->probabilityWeight;

    std::uniform_int_distribution<int> weightDist(0, totalWeight - 1);
    int roll = weightDist(rng);

    int cumulative = 0;
    RandomEvent* selected = available[0];
    for (auto* e : available) {
        cumulative += e->probabilityWeight;
        if (roll < cumulative) {
            selected = e;
            break;
        }
    }

    ConsoleUI::PrintSeparator();
    ConsoleUI::PrintHeader(selected->name);
    std::cout << selected->description << "\n";
    selected->effect(player);
    player.getStats().clampAll();

    return true;
}
