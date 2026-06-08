#pragma once
#include "GameState.h"
#include "../npc/NPC.h"
#include "../events/RandomEvent.h"
#include "../exams/Exam.h"
#include "../save/SaveManager.h"
#include "../systems/EventJournal.h"
#include "../systems/Encyclopedia.h"
#include "../systems/Achievements.h"
#include "../systems/Habits.h"
#include "../data/Difficulty.h"
#include <memory>
#include <map>
#include <string>

class Game {
private:
    DifficultyLevel selectDifficulty();
    void applyDifficultyStartSettings(DifficultyLevel difficulty);

private:
    GameState state;
    std::unique_ptr<Alla> alla;
    std::unique_ptr<Bulat> bulat;
    std::unique_ptr<Semen> semen;
    std::unique_ptr<Artem> artem;
    RandomEventManager eventManager;
    EventJournal journal;
    Encyclopedia encyclopedia;
    AchievementSystem achievements;
    HabitSystem habits;
    bool running = true;

    void initGame();
    void initNPCs();

    void runPrologue();
    void runDay1();
    void runDay2();
    void runDay3();
    void runDay4();
    void runDay5();
    void runDay6();
    void runDay7();
    void runDay8();

    void offerHomeMeal(const std::string& title, bool beforeSleep);

    void handleLocation(LocationID loc);
    void handleHomeLocation();
    void handleUniversityLocation();
    void handleStreetLocation();
    void handleCanteenLocation();
    void handleShopLocation();
    void handleFlowerShopLocation();

    void interactWithAlla();
    void interactWithBulat();
    void interactWithSemen();
    void interactWithArtem();
    void recordNPCChoice(const std::string& npcName, const std::string& choiceKey, int value = 1);

    int takeExam(Exam& exam, int examId);

    void saveGame();
    void loadGame();

    void applyDailySystems();
    void checkGameOver();
    void checkAchievements();

    void showJournal();
    void showEncyclopedia();
    void showAchievements();
    void showHabits();

    void modifyHiddenStat(const std::string& stat, int delta);

public:
    Game();
    void run();
};
