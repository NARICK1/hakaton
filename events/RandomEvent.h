#pragma once
#include "../player/Player.h"
#include <string>
#include <functional>

struct RandomEvent {
    std::string name;
    std::string description;
    std::function<void(Player&)> effect;
    int probabilityWeight = 10;
    int minDay = 1;
    int maxDay = 8;
};

class RandomEventManager {
private:
    std::vector<RandomEvent> events;
    void initEvents();

public:
    RandomEventManager();
    bool tryTriggerEvent(Player& player, int day);
};
