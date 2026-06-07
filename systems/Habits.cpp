#include "Habits.h"
#include <sstream>
#include <algorithm>

HabitSystem::HabitSystem() {
    habits["study_daily"] = {"study_daily", "Ежедневная учёба",
        "Заниматься каждый день", "study", 0, 0, 0, "0", true};
    habits["no_junk"] = {"no_junk", "Здоровое питание",
        "Не есть фастфуд", "health", 0, 0, 0, "0", true};
    habits["walk_evening"] = {"walk_evening", "Вечерняя прогулка",
        "Гулять перед сном", "health", 0, 0, 0, "0", true};
    habits["socialize"] = {"socialize", "Общение",
        "Говорить хотя бы с одним NPC", "social", 0, 0, 0, "0", true};
    habits["procrastinate"] = {"procrastinate", "Прокрастинация",
        "Откладывать дела на потом", "lifestyle", 0, 0, 0, "0", false};
    habits["junk_food"] = {"junk_food", "Фастфуд",
        "Есть вредную еду", "health", 0, 0, 0, "0", false};
    habits["skip_classes"] = {"skip_classes", "Прогулы",
        "Пропускать пары", "study", 0, 0, 0, "0", false};
}

void HabitSystem::recordDay(const std::string& habitId, int dayNum) {
    auto it = habits.find(habitId);
    if (it == habits.end()) return;
    auto& h = it->second;
    std::string dayStr = std::to_string(dayNum);
    if (h.lastDoneDay != dayStr) {
        if (h.lastDoneDay == std::to_string(dayNum - 1)) {
            h.streak++;
        } else {
            h.streak = 1;
        }
        h.lastDoneDay = dayStr;
        h.totalDays++;
        if (h.streak > h.bestStreak) h.bestStreak = h.streak;
    }
}

void HabitSystem::breakStreak(const std::string& habitId) {
    auto it = habits.find(habitId);
    if (it != habits.end()) {
        it->second.streak = 0;
    }
}

const Habit* HabitSystem::getHabit(const std::string& id) const {
    auto it = habits.find(id);
    if (it != habits.end()) return &it->second;
    return nullptr;
}

std::vector<const Habit*> HabitSystem::getAllHabits() const {
    std::vector<const Habit*> result;
    for (const auto& [id, h] : habits) {
        result.push_back(&h);
    }
    return result;
}

std::vector<const Habit*> HabitSystem::getByCategory(const std::string& cat) const {
    std::vector<const Habit*> result;
    for (const auto& [id, h] : habits) {
        if (h.category == cat) result.push_back(&h);
    }
    return result;
}

std::string HabitSystem::getStreakString(const std::string& habitId) const {
    auto it = habits.find(habitId);
    if (it == habits.end()) return "";
    const auto& h = it->second;
    return h.title + ": " + std::to_string(h.streak) + " дн. (рекорд: " +
           std::to_string(h.bestStreak) + ")";
}

std::string HabitSystem::serialize() const {
    std::ostringstream oss;
    oss << habits.size() << "\n";
    for (const auto& [id, h] : habits) {
        oss << id << " " << h.streak << " " << h.bestStreak << " "
            << h.totalDays << " " << h.lastDoneDay << "\n";
    }
    return oss.str();
}

bool HabitSystem::deserialize(const std::string& data) {
    std::istringstream iss(data);
    size_t count;
    if (!(iss >> count)) return false;
    for (size_t i = 0; i < count; i++) {
        std::string id;
        Habit h;
        if (!(iss >> id >> h.streak >> h.bestStreak >> h.totalDays >> h.lastDoneDay)) return false;
        auto it = habits.find(id);
        if (it != habits.end()) {
            it->second.streak = h.streak;
            it->second.bestStreak = h.bestStreak;
            it->second.totalDays = h.totalDays;
            it->second.lastDoneDay = h.lastDoneDay;
        }
    }
    return true;
}
