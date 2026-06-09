#pragma once
#include <string>
#include <vector>
#include <ctime>

struct JournalEntry {
    int day;
    std::string location;
    std::string description;
    std::string category; // "story", "event", "npc", "exam", "achievement"
    int importance; // 1-5

    std::string toString() const;
};

class EventJournal {
private:
    std::vector<JournalEntry> entries;
    int maxEntries = 50;
public:
    void addEntry(int day, const std::string& loc, const std::string& desc,
                  const std::string& cat = "story", int importance = 3);
    const std::vector<JournalEntry>& getAll() const { return entries; }
    std::vector<JournalEntry> getByCategory(const std::string& cat) const;
    std::vector<JournalEntry> getRecent(int count = 10) const;
    void clear();
    std::string serialize() const;
    bool deserialize(const std::string& data);
};
