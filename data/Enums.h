#pragma once
#include <string>

enum class DayOfWeek {
    Monday = 1,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

enum class LocationID {
    Home,
    University,
    Street,
    Canteen,
    Shop,
    FlowerShop
};

enum class NPCState {
    Neutral,
    Friendly,
    Hostile
};

enum class RelationshipLevel {
    Enemy = 0,
    Bad = 20,
    Neutral = 40,
    Good = 60,
    Great = 80,
    Max = 100
};

enum class ExamResult {
    NotTaken,
    Failed,
    Passed,
    Excellent
};

enum class GameOverCondition {
    None,
    SuperEnding,
    GoodEnding,
    NormalEnding,
    Expelled,
    AcademicLeave,
    MentalHospital,
    StarvedToDeath,
    EternalDebtor,
    SecretRomantic,
    SecretArmy
};

enum class DifficultyLevel {
    Easy = 1,
    Normal = 2,
    Hard = 3
};

inline std::string difficultyToString(DifficultyLevel difficulty) {
    switch (difficulty) {
    case DifficultyLevel::Easy: return "Изи мод";
    case DifficultyLevel::Normal: return "Нормальный";
    case DifficultyLevel::Hard: return "Хард";
    default: return "Нормальный";
    }
}

enum class GamePhase {
    MainMenu,
    Playing,
    ExamInProgress,
    GameOver,
    SaveLoad
};

enum class BuffType {
    ImposterSyndrome,
    Burnout,
    BrokenHeart,
    SleepParalysis,
    Starvation
};

enum class ChoiceEffect {
    None,
    IncreaseIntellect,
    DecreaseIntellect,
    IncreaseEnergy,
    DecreaseEnergy,
    IncreaseStress,
    DecreaseStress,
    IncreaseMoney,
    DecreaseMoney,
    IncreaseRomance,
    DecreaseRomance,
    IncreaseAllaRelation,
    DecreaseAllaRelation,
    IncreaseBulatRelation,
    DecreaseBulatRelation,
    IncreaseSemenRelation,
    DecreaseSemenRelation,
    IncreaseTeacherRelation,
    DecreaseTeacherRelation,
    IncreaseHumanity,
    DecreaseHumanity,
    IncreaseHunger,
    DecreaseHunger
};

inline std::string locationToString(LocationID loc) {
    switch (loc) {
    case LocationID::Home: return "Дом";
    case LocationID::University: return "Университет";
    case LocationID::Street: return "Улица";
    case LocationID::Canteen: return "Столовая";
    case LocationID::Shop: return "Магазин";
    case LocationID::FlowerShop: return "Цветочный магазин";
    default: return "Неизвестно";
    }
}

inline std::string gameOverToString(GameOverCondition cond) {
    switch (cond) {
    case GameOverCondition::SuperEnding: return "Супер-концовка";
    case GameOverCondition::GoodEnding: return "Хорошая концовка";
    case GameOverCondition::NormalEnding: return "Нормальная концовка";
    case GameOverCondition::Expelled: return "Отчисление";
    case GameOverCondition::AcademicLeave: return "Академ";
    case GameOverCondition::MentalHospital: return "Психбольница";
    case GameOverCondition::StarvedToDeath: return "Смерть от голода";
    case GameOverCondition::EternalDebtor: return "Вечный должник";
    case GameOverCondition::SecretRomantic: return "Секретная романтическая концовка";
    case GameOverCondition::SecretArmy: return "Секретная армейская концовка";
    default: return "Неизвестно";
    }
}
