#pragma once
#include "../player/Player.h"
#include "../data/Enums.h"
#include <string>

class GameState {
private:
    Player player;
    GamePhase currentPhase = GamePhase::MainMenu;
    GameOverCondition gameOverReason = GameOverCondition::None;
    int currentDay = 1;
    bool gameStarted = false;
    std::string npcMemoryData; // serialized NPC memory for save/load
    std::string journalData;
    std::string encyclopediaData;
    std::string achievementsData;
    std::string habitsData;

public:
    GameState() = default;

    // Геттеры/сеттеры
    Player& getPlayer() { return player; }
    const Player& getPlayer() const { return player; }
    GamePhase getPhase() const { return currentPhase; }
    void setPhase(GamePhase phase) { currentPhase = phase; }
    GameOverCondition getGameOverReason() const { return gameOverReason; }
    void setGameOverReason(GameOverCondition reason) { gameOverReason = reason; }
    int getCurrentDay() const { return currentDay; }
    void setCurrentDay(int day) { currentDay = day; }
    bool isGameStarted() const { return gameStarted; }
    void setGameStarted(bool started) { gameStarted = started; }
    const std::string& getNPCMemoryData() const { return npcMemoryData; }
    void setNPCMemoryData(const std::string& data) { npcMemoryData = data; }
    const std::string& getJournalData() const { return journalData; }
    void setJournalData(const std::string& data) { journalData = data; }
    const std::string& getEncyclopediaData() const { return encyclopediaData; }
    void setEncyclopediaData(const std::string& data) { encyclopediaData = data; }
    const std::string& getAchievementsData() const { return achievementsData; }
    void setAchievementsData(const std::string& data) { achievementsData = data; }
    const std::string& getHabitsData() const { return habitsData; }
    void setHabitsData(const std::string& data) { habitsData = data; }

    // Сериализация
    std::string serialize() const;
    bool deserialize(const std::string& data);
};
