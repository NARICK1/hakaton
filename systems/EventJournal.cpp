#include "EventJournal.h"
#include <sstream>
#include <algorithm>

std::string JournalEntry::toString() const {
    std::string stars;
    for (int i = 0; i < importance; i++) stars += "★";
    return "[" + std::to_string(day) + "|" + location + "] " + description + " (" + stars + ")";
}

void EventJournal::addEntry(int day, const std::string& loc, const std::string& desc,
                             const std::string& cat, int importance) {
    JournalEntry e;
    e.day = day;
    e.location = loc;
    e.description = desc;
    e.category = cat;
    e.importance = std::clamp(importance, 1, 5);
    entries.push_back(e);
    if (static_cast<int>(entries.size()) > maxEntries) {
        entries.erase(entries.begin());
    }
}

std::vector<JournalEntry> EventJournal::getByCategory(const std::string& cat) const {
    std::vector<JournalEntry> result;
    for (const auto& e : entries) {
        if (e.category == cat) result.push_back(e);
    }
    return result;
}

std::vector<JournalEntry> EventJournal::getRecent(int count) const {
    std::vector<JournalEntry> result;
    int start = std::max(0, static_cast<int>(entries.size()) - count);
    for (size_t i = static_cast<size_t>(start); i < entries.size(); i++) {
        result.push_back(entries[i]);
    }
    return result;
}

void EventJournal::clear() {
    entries.clear();
}

std::string EventJournal::serialize() const {
    std::ostringstream oss;
    oss << entries.size() << "\n";
    for (const auto& e : entries) {
        oss << e.day << "\n"
            << e.location << "\n"
            << e.description << "\n"
            << e.category << "\n"
            << e.importance << "\n";
    }
    return oss.str();
}

bool EventJournal::deserialize(const std::string& data) {
    std::istringstream iss(data);
    size_t count;
    if (!(iss >> count)) return false;
    iss.ignore();
    entries.clear();
    for (size_t i = 0; i < count; i++) {
        JournalEntry e;
        std::string line;
        if (!std::getline(iss, line)) return false;
        e.day = std::stoi(line);
        if (!std::getline(iss, e.location)) return false;
        if (!std::getline(iss, e.description)) return false;
        if (!std::getline(iss, e.category)) return false;
        if (!(iss >> e.importance)) return false;
        iss.ignore();
        entries.push_back(e);
    }
    return true;
}
