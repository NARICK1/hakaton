#include "Encyclopedia.h"
#include <sstream>
#include <algorithm>

Encyclopedia::Encyclopedia() {
    entries["Алла"] = {
        "Алла", "Студентка-отличница",
        "Алла учится на третьем курсе. С детства мечтала стать программистом, "
        "как её старший брат. Участвовала в олимпиадах по информатике.",
        "Серьёзная, ответственная, но стеснительная. Любит порядок во всём. "
        "Боится публичных выступлений, хотя отлично отвечает у доски.",
        "Дружит с Семёном. Недолюбливает Артёма за его разгильдяйство.",
        "Тайно пишет фанфики по аниме. Стесняется этого."
    };
    entries["Булат"] = {
        "Булат", "Студент-спортсмен",
        "Булат — мастер спорта по лёгкой атлетике. Поступил в университет по "
        "целевому набору от спортивного клуба. Параллельно учится на тренера.",
        "Энергичный, общительный, но вспыльчивый. Не выносит несправедливости. "
        "Всегда заступается за слабых.",
        "В хороших отношениях со всеми, но особенно с Артёмом — однокурсники.",
        "Втайне пишет стихи. Никому не показывает."
    };
    entries["Семён"] = {
        "Семён", "Студент-программист",
        "Семён с 12 лет пишет код. Участвовал в хакатонах, имеет несколько "
        "пет-проектов. Мечтает создать свою IT-компанию.",
        "Интроверт, застенчив, но очень умён. Может говорить часами о технологиях. "
        "Теряется в больших компаниях.",
        "Лучший друг Аллы. Часто делает ей одолжения по учёбе.",
        "Разрабатывает инди-игру в одиночку. Близок к релизу."
    };
    entries["Артём"] = {
        "Артём", "Студент-художник",
        "Артём — творческая личность. Рисует с детства, мечтает стать "
        "иллюстратором. В университете учится на дизайнера.",
        "Весёлый, расслабленный, креативный. Вечно всё откладывает на потом. "
        "Оптимист по жизни, редко унывает.",
        "Дружит с Булатом. Часто подшучивает над Семёном.",
        "Подрабатывает созданием мемов и стикеров для телеграм-каналов."
    };
    entries["Преподаватели"] = {
        "Преподаватели", "Профессорско-преподавательский состав",
        "В университете преподают опытные педагоги. Самый строгий — профессор "
        "математики, самый добрый — преподаватель физкультуры.",
        "Разные характеры, но всех объединяет любовь к своему делу.",
        "Уважают прилежных студентов. Не терпят списывания.",
        "Профессор математики — фанат головоломок и кроссвордов."
    };
}

void Encyclopedia::discover(const std::string& npcName) {
    auto it = entries.find(npcName);
    if (it != entries.end()) {
        it->second.discovered = true;
    }
}

bool Encyclopedia::isDiscovered(const std::string& npcName) const {
    auto it = entries.find(npcName);
    return it != entries.end() && it->second.discovered;
}

const NPCLore* Encyclopedia::getEntry(const std::string& npcName) const {
    auto it = entries.find(npcName);
    if (it != entries.end()) return &it->second;
    return nullptr;
}

std::vector<std::string> Encyclopedia::getDiscoveredList() const {
    std::vector<std::string> result;
    for (const auto& [name, lore] : entries) {
        if (lore.discovered) result.push_back(name);
    }
    return result;
}

std::vector<std::string> Encyclopedia::getAllNames() const {
    std::vector<std::string> result;
    for (const auto& [name, lore] : entries) {
        result.push_back(name);
    }
    return result;
}

std::string NPCLore::formatEntry() const {
    std::string result;
    result += "=== " + name + " ===";
    result += "\nРоль: " + role;
    result += "\n\n" + backstory;
    result += "\n\nХарактер: " + personality;
    result += "\n\nОтношения: " + knownRelations;
    result += "\n\nСекрет: " + secrets;
    return result;
}

std::string Encyclopedia::serialize() const {
    std::ostringstream oss;
    oss << entries.size() << "\n";
    for (const auto& [name, lore] : entries) {
        oss << name << " " << (lore.discovered ? "1" : "0") << "\n";
    }
    return oss.str();
}

bool Encyclopedia::deserialize(const std::string& data) {
    std::istringstream iss(data);
    size_t count;
    if (!(iss >> count)) return false;
    for (size_t i = 0; i < count; i++) {
        std::string name;
        int disc;
        if (!(iss >> name >> disc)) return false;
        auto it = entries.find(name);
        if (it != entries.end()) {
            it->second.discovered = (disc == 1);
        }
    }
    return true;
}
