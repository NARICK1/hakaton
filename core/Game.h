#pragma once
#include "GameState.h"
#include "../npc/NPC.h"
#include "../events/RandomEvent.h"
#include "../exams/Exam.h"
#include "../save/SaveManager.h"
#include <memory>
#include <map>

class Game {
private:
    GameState state;
    std::unique_ptr<Alla> alla;
    std::unique_ptr<Bulat> bulat;
    std::unique_ptr<Semen> semen;
    std::unique_ptr<Artem> artem;
    RandomEventManager eventManager;
    bool running = true;

    void initGame();
    void initNPCs();

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

public:
    Game();
    void run();
};
