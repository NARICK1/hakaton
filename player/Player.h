#pragma once
#include "Stats.h"
#include "../data/Enums.h"
#include "../data/GameConfig.h"
#include <string>
#include <map>
#include <vector>

class Player {
private:
    std::string name;
    Stats stats;
    std::map<int, int> grades;
    int debts = 0;
    int currentDay = 1;
    int currentHour = GameConstants::START_HOUR;
    int currentMinute = GameConstants::START_MINUTE;
    LocationID currentLocation = LocationID::Home;
    std::map<std::string, bool> flags;
    std::vector<BuffType> activeBuffs;

    // Отношения с NPC
    std::map<std::string, int> npcRelationships;

public:
    Player() = default;
    explicit Player(const std::string& playerName);

    // Геттеры
    const std::string& getName() const { return name; }
    Stats& getStats() { return stats; }
    const Stats& getStats() const { return stats; }
    int getGrade(int examId) const;
    void setGrade(int examId, int grade);
    int getDebts() const { return debts; }
    void addDebt(int count = 1);
    void removeDebt(int count = 1);
    int getCurrentDay() const { return currentDay; }
    void nextDay();
    int getCurrentHour() const { return currentHour; }
    int getCurrentMinute() const { return currentMinute; }
    LocationID getLocation() const { return currentLocation; }
    void setLocation(LocationID loc) { currentLocation = loc; }

    // Время
    void advanceTime(int minutes);
    std::string getTimeString() const;

    // Флаги
    void setFlag(const std::string& key, bool value = true);
    bool hasFlag(const std::string& key) const;

    // Отношения
    int getRelation(const std::string& npcName) const;
    void modifyRelation(const std::string& npcName, int delta);

    // Баффы
    void addBuff(BuffType buff);
    void removeBuff(BuffType buff);
    bool hasBuff(BuffType buff) const;
    const std::vector<BuffType>& getActiveBuffs() const { return activeBuffs; }

    // Состояние
    bool isAlive() const { return stats.isAlive(); }
    void applyBuffs();

    // Сохранение/загрузка
    std::string serialize() const;
    bool deserialize(const std::string& data);
};
