#pragma once
#include <string>

namespace AsciiArt {

// NPC portraits
std::string GetAlla();
std::string GetBulat();
std::string GetSemen();
std::string GetArtem();

// Locations
std::string GetHome();
std::string GetUniversity();
std::string GetClassroom();
std::string GetCafeteria();
std::string GetStreet();

// Exams
std::string GetExamTicket();
std::string GetTeacher();

// Weather
std::string GetRain();
std::string GetSun();

// Endings
std::string GetSuccessEnding();
std::string GetFailEnding();

// Fallback
std::string GetDefaultArt();

// Lookup by name
std::string ForNPC(const std::string& npcName);
std::string ForLocation(int locId);
std::string ForExam();
std::string ForWeather(bool isRain);
std::string ForEnding(bool success);
std::string ForDefault();

} // namespace AsciiArt
