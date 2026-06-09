#pragma once
#include "GameState.h"
#include "../npc/NPC.h"
#include "../events/RandomEvent.h"
#include "../exams/Exam.h"
#include "../save/SaveManager.h"
#include "../data/Difficulty.h"
#include "../systems/Achievements.h"
#include "../systems/EventJournal.h"
#include "../systems/Encyclopedia.h"
#include "../systems/Habits.h"
#include <memory>
#include <map>
#include <string>

class Game {
private:
    GameState state;
    std::unique_ptr<Alla> alla;
    std::unique_ptr<Bulat> bulat;
    std::unique_ptr<Semen> semen;
    std::unique_ptr<Artem> artem;
    RandomEventManager eventManager;
    AchievementSystem achievements;
    EventJournal journal;
    Encyclopedia encyclopedia;
    HabitSystem habits;
    bool running = true;

    void initGame();
    void initNPCs();
    DifficultyLevel selectDifficulty();
    void applyDifficultyStartSettings(DifficultyLevel difficulty);

    // Пролог и сцены по дням
    void runPrologue();
    void runDay1();
    void runDay2();
    void runDay3();
    void runDay4();
    void runDay5();
    void runDay6();
    void runDay7();
    void runDay8();

    // Дополнительные сцены дней 6-7
    void runDay6DatePreparation();
    void runDateWithAlla();
    void runRetakeExam();
    void runNormalDay7();

    // Еда дома утром / вечером
    void offerHomeMeal(const std::string& title, bool beforeSleep);

    // Системы локаций
    void handleLocation(LocationID loc);
    void handleHomeLocation();
    void handleUniversityLocation();
    void handleStreetLocation();
    void handleCanteenLocation();
    void handleShopLocation();
    void handleFlowerShopLocation();

    // Взаимодействие с NPC
    void interactWithAlla();
    void interactWithBulat();
    void interactWithSemen();
    void interactWithArtem();
    void recordNPCChoice(const std::string& npcName, const std::string& choiceKey, int value = 1);

    // Экзамены
    int takeExam(Exam& exam, int examId);

    // Сохранение/загрузка
    void saveGame();
    void loadGame();

    // Системы
    void applyDailySystems();
    void checkGameOver();
    void checkAchievements();
    void showAchievements();
    void showJournal();
    void showEncyclopedia();
    void showHabits();
    void addJournalEntry(const std::string& location,
                         const std::string& description,
                         const std::string& category = "story",
                         int importance = 3);

public:
    Game();
    void run();
};