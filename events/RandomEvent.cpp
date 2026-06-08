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
                int gain = p.scaleMoneyGain(200);
                p.getStats().money += gain;
                p.getStats().clampAll();
                std::cout << "+" << gain << " рублей!\n";
            },
            15, 1, 8
        },
        {
            "Потеря",
            "Ты потерял деньги из кармана.",
            [](Player& p) {
                auto& s = p.getStats();
                int loss = p.scaleCost(150);
                s.money = std::max(0, s.money - loss);
                s.clampAll();
                std::cout << "-" << loss << " рублей...\n";
            },
            10, 1, 8
        },
        {
            "Встреча знакомого",
            "На улице ты встретил старого друга. Он подбодрил тебя.",
            [](Player& p) {
                auto& s = p.getStats();
                int stressDown = p.scaleGain(10);
                int energyGain = p.scaleGain(5);
                s.stress = std::max(0, s.stress - stressDown);
                s.energy += energyGain;
                s.clampAll();
                std::cout << "Стресс -" << stressDown << ", энергия +" << energyGain << "\n";
            },
            12, 1, 8
        },
        {
            "Хорошая еда",
            "В столовой сегодня дают бесплатные обеды!",
            [](Player& p) {
                auto& s = p.getStats();

                int hungerGain = p.scaleGain(30);
                int energyGain = p.scaleGain(10);

                // hunger теперь сытость
                s.hunger = std::min(GameConstants::MAX_HUNGER, s.hunger + hungerGain);
                s.energy += energyGain;

                s.clampAll();
                std::cout << "Сытость +" << hungerGain << ", энергия +" << energyGain << "\n";
            },
            8, 1, 8
        },
        {
            "Плохая еда",
            "Ты съел что-то не то в столовой. Теперь плохо.",
            [](Player& p) {
                auto& s = p.getStats();

                int hungerLoss = p.scalePenalty(10);
                int energyLoss = p.scalePenalty(10);
                int healthLoss = p.scalePenalty(5);

                // Плохая еда снижает сытость и здоровье
                s.hunger = std::max(0, s.hunger - hungerLoss);
                s.energy -= energyLoss;
                s.health -= healthLoss;

                s.clampAll();
                std::cout << "Сытость -" << hungerLoss
                          << ", энергия -" << energyLoss
                          << ", здоровье -" << healthLoss << "\n";
            },
            8, 1, 8
        },
        {
            "Помощь преподавателя",
            "Преподаватель заметил твои старания и предложил дополнительное занятие.",
            [](Player& p) {
                auto& s = p.getStats();
                int intellectGain = p.scaleGain(5);
                s.intellect += intellectGain;
                p.modifyRelation("Преподаватели", 5);
                s.clampAll();
                std::cout << "Интеллект +" << intellectGain << ", отношения с преподавателями улучшились\n";
            },
            10, 1, 8
        },
        {
            "Анекдот на экзамене",
            "Ты рассказал анекдот, и преподаватель посмеялся. Настроение улучшилось!",
            [](Player& p) {
                auto& s = p.getStats();
                int stressDown = p.scaleGain(15);
                s.stress = std::max(0, s.stress - stressDown);
                p.modifyRelation("Преподаватели", 3);
                s.clampAll();
                std::cout << "Стресс -" << stressDown << ", отношения с преподавателями улучшились\n";
            },
            7, 1, 8
        },
        {
            "Сосед-музыкант",
            "Сосед всю ночь играл на гитаре. Ты не выспался.",
            [](Player& p) {
                auto& s = p.getStats();
                int energyLoss = p.scalePenalty(15);
                int fatigueGain = p.scalePenalty(15);
                int stressGain = p.scalePenalty(5);
                s.energy -= energyLoss;
                s.fatigue += fatigueGain;
                s.stress += stressGain;
                s.clampAll();
                std::cout << "Энергия -" << energyLoss
                          << ", усталость +" << fatigueGain
                          << ", стресс +" << stressGain << "\n";
            },
            10, 1, 8
        },
        {
            "Удачная покупка",
            "Ты купил товары со скидкой и сэкономил 200 рублей!",
            [](Player& p) {
                int gain = p.scaleMoneyGain(200);
                p.getStats().money += gain;
                p.getStats().clampAll();
                std::cout << "+" << gain << " рублей!\n";
            },
            8, 1, 8
        },
        {
            "Звонок родителей",
            "Родители позвонили и подбодрили тебя. Ты чувствуешь прилив сил.",
            [](Player& p) {
                auto& s = p.getStats();
                int energyGain = p.scaleGain(10);
                int stressDown = p.scaleGain(10);
                s.energy += energyGain;
                s.stress = std::max(0, s.stress - stressDown);
                s.clampAll();
                std::cout << "Энергия +" << energyGain << ", стресс -" << stressDown << "\n";
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
