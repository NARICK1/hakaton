#pragma once

namespace GameConstants {
    // Начальные значения
    constexpr int START_INTELLECT = 50;
    constexpr int START_ENERGY = 100;
    constexpr int START_FATIGUE = 0;
    constexpr int START_HUNGER = 0;
    constexpr int START_STRESS = 10;
    constexpr int START_HUMANITY = 70;
    constexpr int START_MONEY = 1000;
    constexpr int START_ROMANCE = 0;
    constexpr int START_RELATIONSHIP = 50;

    // Пороговые значения
    constexpr int MAX_STAT = 100;
    constexpr int MIN_STAT = 0;
    constexpr int MAX_MONEY = 9999;
    constexpr int MAX_HUNGER = 100;
    constexpr int MAX_FATIGUE = 100;
    constexpr int STARVATION_THRESHOLD = 90;
    constexpr int FATIGUE_CRITICAL = 80;
    constexpr int STRESS_CRITICAL = 80;

    // Время
    constexpr int START_HOUR = 8;
    constexpr int START_MINUTE = 0;
    constexpr int TOTAL_DAYS = 8;
    constexpr int MINUTES_PER_HOUR = 60;
    constexpr int HOURS_PER_DAY = 24;

    // Эффекты действий
    constexpr int STUDY_INTELLECT_GAIN = 3;
    constexpr int STUDY_FATIGUE_COST = 10;
    constexpr int STUDY_STRESS_COST = 5;
    constexpr int STUDY_ENERGY_COST = 15;

    constexpr int SLEEP_ENERGY_GAIN = 60;
    constexpr int SLEEP_FATIGUE_REDUCE = 50;
    constexpr int SLEEP_STRESS_REDUCE = 15;
    constexpr int SLEEP_HUNGER_INCREASE = 20;

    constexpr int EAT_HUNGER_REDUCE = 40;
    constexpr int EAT_ENERGY_GAIN = 10;
    constexpr int EAT_MONEY_COST = 100;

    constexpr int WALK_STRESS_REDUCE = 5;
    constexpr int WALK_FATIGUE_COST = 5;

    // Отношения
    constexpr int RELATIONSHIP_BOOST_HELP = 10;
    constexpr int RELATIONSHIP_BOOST_GIFT = 15;
    constexpr int RELATIONSHIP_BOOST_COMPLIMENT = 5;
    constexpr int RELATIONSHIP_PENALTY_RUDE = -15;
    constexpr int RELATIONSHIP_PENALTY_IGNORE = -5;

    // Деньги
    constexpr int MONEY_PER_DAY_SCHOLARSHIP = 200;
    constexpr int FLOWER_COST = 300;

    // Проходной балл
    constexpr int EXAM_PASS_THRESHOLD = 50;
    constexpr int EXAM_EXCELLENT_THRESHOLD = 80;
}
