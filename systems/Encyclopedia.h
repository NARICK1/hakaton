#pragma once
#include <string>
#include <vector>
#include <map>

struct NPCLore {
    std::string name;
    std::string role;
    std::string backstory;
    std::string personality;
    std::string knownRelations;
    std::string secrets;
    bool discovered = false;

    std::string formatEntry() const;
};

class Encyclopedia {
private:
    std::map<std::string, NPCLore> entries;
public:
    Encyclopedia();
    void discover(const std::string& npcName);
    bool isDiscovered(const std::string& npcName) const;
    const NPCLore* getEntry(const std::string& npcName) const;
    std::vector<std::string> getDiscoveredList() const;
    std::vector<std::string> getAllNames() const;
    std::string serialize() const;
    bool deserialize(const std::string& data);
};
