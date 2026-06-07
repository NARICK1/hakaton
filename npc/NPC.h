#pragma once
#include "../data/GameConfig.h"
#include "../data/Enums.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>

class Player;

class NPC {
protected:
    std::string name;
    std::string description;
    std::map<std::string, NPCMemoryEntry> memory;
    std::vector<NPCAction> actionHistory;
    int relationship = 50;   // общее отношение
    int trustLevel = 50;     // доверие
    int respectLevel = 50;   // уважение
    int sympathyLevel = 50;  // симпатия
    int interestLevel = 50;  // интерес
    int irritationLevel = 10; // раздражение

public:
    NPC() = default;
    NPC(const std::string& npcName, const std::string& desc);
    virtual ~NPC() = default;

    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    int getRelationship() const { return relationship; }
    int getTrust() const { return trustLevel; }
    int getRespect() const { return respectLevel; }
    int getSympathy() const { return sympathyLevel; }
    int getInterest() const { return interestLevel; }
    int getIrritation() const { return irritationLevel; }
    void setRelationship(int val) { relationship = std::clamp(val, 0, 100); }
    void setTrust(int val) { trustLevel = std::clamp(val, 0, 100); }
    void setRespect(int val) { respectLevel = std::clamp(val, 0, 100); }
    void setSympathy(int val) { sympathyLevel = std::clamp(val, 0, 100); }
    void setInterest(int val) { interestLevel = std::clamp(val, 0, 100); }
    void setIrritation(int val) { irritationLevel = std::clamp(val, 0, 100); }
    void modifyRelationship(int delta);
    void modifyTrust(int delta);
    void modifyRespect(int delta);
    void modifySympathy(int delta);
    void modifyInterest(int delta);
    void modifyIrritation(int delta);

    // Система памяти NPC — простые ключи
    void rememberEvent(const std::string& eventKey, int value = 1);
    bool hasMemory(const std::string& eventKey) const;
    int getMemoryValue(const std::string& eventKey) const;
    bool wasHelped() const;
    bool wasRude() const;
    int getHelpCount() const;

    // Расширенная память — история действий
    void recordAction(const NPCAction& action);
    const std::vector<NPCAction>& getActionHistory() const { return actionHistory; }
    int countActionsByCategory(const std::string& category) const;
    bool hasPositiveHistory(int currentDay, int recentDays = 3) const;
    bool hasNegativeHistory(int currentDay, int recentDays = 3) const;
    std::string getEmotionalState() const;

    // Диалоги
    virtual std::string getDialog(const Player& player) const;
    virtual std::vector<Choice> getChoices(const Player& player) const;

    virtual std::string serialize() const;
    virtual bool deserialize(const std::string& data);
};

// Конкретные NPC
class Alla : public NPC {
public:
    Alla();
    std::string getDialog(const Player& player) const override;
    std::vector<Choice> getChoices(const Player& player) const override;
};

class Bulat : public NPC {
public:
    Bulat();
    std::string getDialog(const Player& player) const override;
    std::vector<Choice> getChoices(const Player& player) const override;
};

class Semen : public NPC {
public:
    Semen();
    std::string getDialog(const Player& player) const override;
    std::vector<Choice> getChoices(const Player& player) const override;
};

class Artem : public NPC {
public:
    Artem();
    std::string getDialog(const Player& player) const override;
    std::vector<Choice> getChoices(const Player& player) const override;
};
