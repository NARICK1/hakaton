#pragma once

#include "Enums.h"
#include <algorithm>
#include <string>

namespace DifficultyRules {
    inline int ApplyPercent(int value, int percent) {
        if (value <= 0) {
            return 0;
        }

        int result = (value * percent + 50) / 100;

        if (percent > 0 && result <= 0) {
            result = 1;
        }

        return result;
    }

    inline int PositiveGain(DifficultyLevel difficulty, int value) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return ApplyPercent(value, 150);
        case DifficultyLevel::Hard:
            return ApplyPercent(value, 60);
        case DifficultyLevel::Normal:
        default:
            return value;
        }
    }

    inline int Penalty(DifficultyLevel difficulty, int value) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return ApplyPercent(value, 50);
        case DifficultyLevel::Hard:
            return ApplyPercent(value, 180);
        case DifficultyLevel::Normal:
        default:
            return value;
        }
    }

    inline int MoneyGain(DifficultyLevel difficulty, int value) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return ApplyPercent(value, 150);
        case DifficultyLevel::Hard:
            return ApplyPercent(value, 70);
        case DifficultyLevel::Normal:
        default:
            return value;
        }
    }

    inline int MoneyCost(DifficultyLevel difficulty, int value) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return ApplyPercent(value, 70);
        case DifficultyLevel::Hard:
            return ApplyPercent(value, 140);
        case DifficultyLevel::Normal:
        default:
            return value;
        }
    }

    inline int ExamScoreModifier(DifficultyLevel difficulty) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return 12;
        case DifficultyLevel::Hard:
            return -18;
        case DifficultyLevel::Normal:
        default:
            return 0;
        }
    }

    inline int StartingMoney(DifficultyLevel difficulty, int normalMoney) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return normalMoney + 500;
        case DifficultyLevel::Hard:
            return std::max(0, normalMoney - 400);
        case DifficultyLevel::Normal:
        default:
            return normalMoney;
        }
    }

    inline int StartingStress(DifficultyLevel difficulty, int normalStress) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return std::max(0, normalStress - 8);
        case DifficultyLevel::Hard:
            return std::min(100, normalStress + 15);
        case DifficultyLevel::Normal:
        default:
            return normalStress;
        }
    }

    inline int StartingFatigue(DifficultyLevel difficulty, int normalFatigue) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return std::max(0, normalFatigue - 10);
        case DifficultyLevel::Hard:
            return std::min(100, normalFatigue + 10);
        case DifficultyLevel::Normal:
        default:
            return normalFatigue;
        }
    }

    inline int StartingIntellect(DifficultyLevel difficulty, int normalIntellect) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return std::min(100, normalIntellect + 10);
        case DifficultyLevel::Hard:
            return std::max(0, normalIntellect - 10);
        case DifficultyLevel::Normal:
        default:
            return normalIntellect;
        }
    }

    inline std::string Description(DifficultyLevel difficulty) {
        switch (difficulty) {
        case DifficultyLevel::Easy:
            return "Изи мод: больше наград, меньше штрафов, дешёвые покупки, экзамены мягче.";
        case DifficultyLevel::Hard:
            return "Хард: меньше наград, штрафы сильнее, всё дороже, экзамены жёстче. Ошибки реально больно бьют.";
        case DifficultyLevel::Normal:
        default:
            return "Нормальный режим: ресурсы надо менеджерить, но игра не душит за каждую ошибку.";
        }
    }
}
