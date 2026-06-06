#pragma once
#include "../data/GameConfig.h"
#include "../data/Enums.h"
#include <string>
#include <map>
#include <memory>
#include <algorithm>

class Player;

class NPC {
protected:
    std::string name;
    std::string description;
    std::map<std::string, NPCMemoryEntry> memory;
    int relationship = 50;

public:
    NPC() = default;
    NPC(const std::string& npcName, const std::string& desc);
    virtual ~NPC() = default;

    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    int getRelationship() const { return relationship; }
    void setRelationship(int val) { relationship = std::clamp(val, 0, 100); }
    void modifyRelationship(int delta);

    // Система памяти NPC
    void rememberEvent(const std::string& eventKey, int value = 1);
    bool hasMemory(const std::string& eventKey) const;
    int getMemoryValue(const std::string& eventKey) const;
    bool wasHelped() const;
    bool wasRude() const;
    int getHelpCount() const;

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
