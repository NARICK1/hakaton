#pragma once
#include <string>
#include <vector>
#include <map>

struct Achievement {
    std::string id;
    std::string title;
    std::string description;
    bool unlocked = false;
};

class AchievementSystem {
private:
    std::map<std::string, Achievement> achievements;
public:
    AchievementSystem();
    bool unlock(const std::string& id);
    bool isUnlocked(const std::string& id) const;
    const Achievement* getAchievement(const std::string& id) const;
    std::vector<const Achievement*> getUnlocked() const;
    std::vector<const Achievement*> getAll() const;
    int getUnlockedCount() const;
    int getTotalCount() const;
    std::string serialize() const;
    bool deserialize(const std::string& data);
};
