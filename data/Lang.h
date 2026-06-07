#pragma once
#include <string>
#include <map>

class Lang {
public:
    enum Language { Russian, English };

    static void set(Language lang);
    static Language current();
    static std::string currentName();
    static std::string currentNameEn();
    static const std::string& get(const std::string& key);

private:
    static Language& state();
    static std::map<std::string, std::string>& ru();
    static std::map<std::string, std::string>& en();
    static void initRu();
    static void initEn();
};

#define LANG(key) Lang::get(key)
