#include "RandomEvent.h"
#include "../ui/ConsoleUI.h"
#include "../data/Constants.h"
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
            [](Player& p) {
                p.getStats().money += 200;
                p.getStats().clampAll();
                std::cout << "+200 рублей!\n";
            },
            15, 1, 8
        },
        {
            "Потеря",
            "Ты потерял 150 рублей из кармана.",
            [](Player& p) {
                auto& s = p.getStats();
                s.money = std::max(0, s.money - 150);
                s.clampAll();
                std::cout << "-150 рублей...\n";
            },
            10, 1, 8
        },
        {
            "Встреча знакомого",
            "На улице ты встретил старого друга. Он подбодрил тебя.",
            [](Player& p) {
                auto& s = p.getStats();
                s.stress = std::max(0, s.stress - 10);
                s.energy += 5;
                s.clampAll();
                std::cout << "Стресс -10, энергия +5\n";
            },
            12, 1, 8
        },
        {
            "Хорошая еда",
            "В столовой сегодня дают бесплатные обеды!",
            [](Player& p) {
                auto& s = p.getStats();

                // hunger теперь сытость
                s.hunger = std::min(GameConstants::MAX_HUNGER, s.hunger + 30);
                s.energy += 10;

                s.clampAll();
                std::cout << "Сытость +30, энергия +10\n";
            },
            8, 1, 8
        },
        {
            "Плохая еда",
            "Ты съел что-то не то в столовой. Теперь плохо.",
            [](Player& p) {
                auto& s = p.getStats();

                // Плохая еда снижает сытость и здоровье
                s.hunger = std::max(0, s.hunger - 10);
                s.energy -= 10;
                s.health -= 5;

                s.clampAll();
                std::cout << "Сытость -10, энергия -10, здоровье -5\n";
            },
            8, 1, 8
        },
        {
            "Помощь преподавателя",
            "Преподаватель заметил твои старания и предложил дополнительное занятие.",
            [](Player& p) {
                auto& s = p.getStats();
                s.intellect += 5;
                p.modifyRelation("Преподаватели", 5);
                s.clampAll();
                std::cout << "Интеллект +5, отношения с преподавателями +5\n";
            },
            10, 1, 8
        },
        {
            "Анекдот на экзамене",
            "Ты рассказал анекдот, и преподаватель посмеялся. Настроение улучшилось!",
            [](Player& p) {
                auto& s = p.getStats();
                s.stress = std::max(0, s.stress - 15);
                p.modifyRelation("Преподаватели", 3);
                s.clampAll();
                std::cout << "Стресс -15, отношения с преподавателями +3\n";
            },
            7, 1, 8
        },
        {
            "Сосед-музыкант",
            "Сосед всю ночь играл на гитаре. Ты не выспался.",
            [](Player& p) {
                auto& s = p.getStats();
                s.energy -= 15;
                s.fatigue += 15;
                s.stress += 5;
                s.clampAll();
                std::cout << "Энергия -15, усталость +15, стресс +5\n";
            },
            10, 1, 8
        },
        {
            "Удачная покупка",
            "Ты купил товары со скидкой и сэкономил 200 рублей!",
            [](Player& p) {
                p.getStats().money += 200;
                p.getStats().clampAll();
                std::cout << "+200 рублей!\n";
            },
            8, 1, 8
        },
        {
            "Звонок родителей",
            "Родители позвонили и подбодрили тебя. Ты чувствуешь прилив сил.",
            [](Player& p) {
                auto& s = p.getStats();
                s.energy += 10;
                s.stress = std::max(0, s.stress - 10);
                s.clampAll();
                std::cout << "Энергия +10, стресс -10\n";
            },
            12, 1, 8
        }
    };
}

bool RandomEventManager::tryTriggerEvent(Player& player, int day) {
    std::vector<RandomEvent> availableEvents;

    for (const auto& event : events) {
        if (day >= event.minDay && day <= event.maxDay) {
            availableEvents.push_back(event);
        }
    }

    if (availableEvents.empty()) {
        return false;
    }

    std::mt19937 rng(std::random_device{}());

    // 30% шанс, что событие вообще произойдёт
    std::uniform_int_distribution<int> chanceDist(1, 100);
    if (chanceDist(rng) > 30) {
        return false;
    }

    int totalWeight = 0;
    for (const auto& event : availableEvents) {
        totalWeight += event.probabilityWeight;
    }

    std::uniform_int_distribution<int> weightDist(1, totalWeight);
    int roll = weightDist(rng);

    int currentWeight = 0;
    for (const auto& event : availableEvents) {
        currentWeight += event.probabilityWeight;

        if (roll <= currentWeight) {
            ConsoleUI::PrintHeader("СЛУЧАЙНОЕ СОБЫТИЕ");
            std::cout << event.name << "\n";
            std::cout << event.description << "\n\n";

            event.effect(player);
            player.getStats().clampAll();

            ConsoleUI::WaitForEnter();
            return true;
        }
    }

    return false;
}