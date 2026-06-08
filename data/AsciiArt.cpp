#include "AsciiArt.h"
#include "Enums.h"

namespace AsciiArt {

//Добавили корреткное отображение персонжаей

std::string GetAlla() 
{
    return R"(      
     ______________             
    /        o-o   \  
   |                \
   | /\/\/\/\/\/\   |
   |/  0    0    |  |
    \    |      | \/
     \_________/      
        Алла        )";
}
std::string GetBulat() {
    return R"(
   ________________ 
  /  _   / \       \    
  | /  \/    \_/\  \   
  |/ ____________\ | 
    ||____| |____|\/ 
    |      |     | 
     \__________/      
        Булат         )"; 
}

std::string GetSemen() {
    return R"(
      _____________
    /               \ 
    | |\/\/\/\/\/\/\/              
     |/  0    0    |
      |	   |       |
       \__________/ 
            Семён      )";
}

std::string GetArtem() {
    return R"(
      ___________ 
     /           \ 
    |  _     _    |  
    |  0     0    | 
    |    |        |
     \___________/       
        Артём     )";
}

// ---- Locations ----

std::string GetHome() {
    return
        "      /\\              \n"
        "     /  \\             \n"
        "    / __ \\            \n"
        "   / /  \\ \\           \n"
        "  /_/    \\_\\          \n"
        "  |  []   |           \n"
        "  |  __   |           \n"
        "  | |  |  |           \n"
        "  |_|__|__|           \n"
        "   Дом                \n";
}

std::string GetUniversity() {
    return
        "  __________________ \n"
        " | []  []  []  []  | \n"
        " | []  []  []  []  | \n"
        " |       __        | \n"
        " |      |  |       | \n"
        " |______|__|_______| \n"
        "   Университет       \n";
}

std::string GetClassroom() {
    return
        "  __________________ \n"
        " |  ___________     | \n"
        " | |  DOSKA    |    | \n"
        " | |___________|    | \n"
        " | []    []    []   | \n"
        " | []    []    []   | \n"
        " |_________________| \n"
        "   Аудитория        \n";
}

std::string GetCafeteria() {
    return
        "  __________________ \n"
        " | [==]   [==]      | \n"
        " | [==]   [==]      | \n"
        " |   ________       | \n"
        " |  |  EDA   |      | \n"
        " |  |________|      | \n"
        " |_________________| \n"
        "   Столовая         \n";
}

std::string GetStreet() {
    return
        "      __________     \n"
        "     /          \\    \n"
        "    /____________\\   \n"
        "       |    |        \n"
        "    ___|____|___     \n"
        "   |           |     \n"
        "   |___________|     \n"
        "   Улица             \n";
}

// ---- Exams ----

std::string GetExamTicket() {
    return
        "   _______________   \n"
        "  |   БИЛЕТ #     |  \n"
        "  |---------------|  \n"
        "  |  Вопрос 1     |  \n"
        "  |  Вопрос 2     |  \n"
        "  |  Вопрос 3     |  \n"
        "  |_______________|  \n"
        "   Экзамен            \n";
}

std::string GetTeacher() {
    return
        "   _______________   \n"
        "  |  .---------.   |  \n"
        "  | (  o    o  )  |  \n"
        "  |  '---v---'    |  \n"
        "  |    /   \\      |  \n"
        "  |   /     \\     |  \n"
        "  |_______________|  \n"
        "   Преподаватель      \n";
}

// ---- Weather ----

std::string GetRain() {
    return
        "   .-~~~~~~~-.       \n"
        "  (           )      \n"
        "   '.-___.-'        \n"
        "      | | |          \n"
        "    | | | | |        \n"
        "      | | |          \n"
        "   Дождь              \n";
}

std::string GetSun() {
    return
        "     \\ | | /         \n"
        "      \\| |/          \n"
        "    .-- O --.        \n"
        "      /| |\\          \n"
        "     / | | \\         \n"
        "   Солнце            \n";
}

// ---- Endings ----

std::string GetSuccessEnding() {
    return
        "   _______________   \n"
        "  /               \\  \n"
        " /   УСПЕШНО!    \\  \n"
        " \\               /  \n"
        "  \\      O      /   \n"
        "    \\    \\|/   /     \n"
        "      \\  | |  /       \n"
        "       \\_____/        \n"
        "   Молодец!          \n";
}

std::string GetFailEnding() {
    return
        "   _______________   \n"
        "  /               \\  \n"
        " /    ПРОВАЛ!     \\  \n"
        " \\               /  \n"
        "  \\      X      /   \n"
        "    \\    \\|/   /     \n"
        "      \\  | |  /       \n"
        "       \\_____/        \n"
        "   Неудача            \n";
}

std::string GetDefaultArt() {
    return
        "   ______________    \n"
        "  /              \\   \n"
        " |  [?]    [?]    |  \n"
        " |  [?]    [?]    |  \n"
        " |                |  \n"
        "  \\              /   \n"
        "   '------------'    \n"
        "   Неизвестно        \n";
}

// ---- Lookup helpers ----

std::string ForNPC(const std::string& npcName) {
    if (npcName == "Alla" || npcName == "Алла") return GetAlla();
    if (npcName == "Bulat" || npcName == "Булат") return GetBulat();
    if (npcName == "Semen" || npcName == "Семён") return GetSemen();
    if (npcName == "Artem" || npcName == "Артём") return GetArtem();
    return GetDefaultArt();
}

std::string ForLocation(int locId) {
    switch (static_cast<LocationID>(locId)) {
    case LocationID::Home: return GetHome();
    case LocationID::University: return GetUniversity();
    case LocationID::Street: return GetStreet();
    case LocationID::Canteen: return GetCafeteria();
    default: return GetDefaultArt();
    }
}

std::string ForExam() {
    return GetExamTicket() + "\n" + GetTeacher();
}

std::string ForWeather(bool isRain) {
    return isRain ? GetRain() : GetSun();
}

std::string ForEnding(bool success) {
    return success ? GetSuccessEnding() : GetFailEnding();
}

std::string ForDefault() {
    return GetDefaultArt();
}

} // namespace AsciiArt
