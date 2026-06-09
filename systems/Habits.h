#pragma once
#include <string>
#include <vector>
#include <map>

struct Habit {
    std::string id;
    std::string title;
    std::string description;
    std::string category; // "study", "social", "health", "lifestyle"
    int streak = 0;
    int bestStreak = 0;
    int totalDays = 0;
    std::string lastDoneDay; // date string or day number

    bool isPositive; // false = bad habit
};

class HabitSystem {
private:
    std::map<std::string, Habit> habits;
public:
    HabitSystem();
    void recordDay(const std::string& habitId, int dayNum);
    void breakStreak(const std::string& habitId);
    const Habit* getHabit(const std::string& id) const;
    std::vector<const Habit*> getAllHabits() const;
    std::vector<const Habit*> getByCategory(const std::string& cat) const;
    std::string getStreakString(const std::string& habitId) const;
    std::string serialize() const;
    bool deserialize(const std::string& data);
};
