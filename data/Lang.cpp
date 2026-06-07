#include "Lang.h"

static std::map<std::string, std::string> _ru;
static std::map<std::string, std::string> _en;
static Lang::Language _lang = Lang::Russian;
static bool _init = false;

static void ensureInit() {
    if (_init) return;
    _init = true;

    // Russian
    _ru["menu_new_game"]     = "Новая игра";
    _ru["menu_load_game"]    = "Загрузить игру";
    _ru["menu_controls"]     = "Управление";
    _ru["menu_settings"]     = "Настройки";
    _ru["menu_exit"]         = "Выход";
    _ru["menu_your_choice"]  = "Ваш выбор";
    _ru["menu_restart"]      = "Начать сначала";
    _ru["menu_back"]         = "Вернуться в меню";
    _ru["menu_lang"]         = "Язык интерфейса";
    _ru["menu_rus"]          = "Русский";
    _ru["menu_eng"]          = "English";
    _ru["menu_profile"]      = "Профиль интерфейса";
    _ru["menu_compact"]      = "Компактный (80 символов)";
    _ru["menu_standard"]     = "Стандартный (120 символов)";
    _ru["menu_wide"]         = "Широкий (140 символов)";
    _ru["menu_back_title"]   = "Назад";
    _ru["menu_title"]        = "БУДНИ СТУДЕНТА";
    _ru["menu_welcome"]      = "Добро пожаловать в Уфимский университет науки и технологий.";
    _ru["menu_welcome2"]     = "Впереди первая сессия, новые знакомства, экзамены и решения,";
    _ru["menu_welcome3"]     = "которые повлияют на всю историю.";
    _ru["menu_current"]      = "Текущий профиль";
    _ru["menu_lang_current"] = "Язык";

    _ru["hud_day"]           = "День";
    _ru["hud_int"]           = "ИНТ";
    _ru["hud_en"]            = "ЭН";
    _ru["hud_fat"]           = "УСТ";
    _ru["hud_hun"]           = "ГОЛ";
    _ru["hud_str"]           = "СТР";
    _ru["hud_money"]         = "РУБ";

    _ru["ui_press_enter"]    = "Нажмите Enter...";
    _ru["ui_your_choice"]    = "Ваш выбор";
    _ru["ui_prompt_choice"]  = "Ваш выбор [%NUM%]:";
    _ru["ui_saved"]          = "Игра сохранена.";
    _ru["ui_loaded"]         = "Сохранение загружено.";
    _ru["ui_invalid"]        = "Некорректный ввод.";
    _ru["ui_save_not_found"] = "Сохранение не найдено.";

    _ru["as_alla"]           = "Алла";
    _ru["as_bulat"]          = "Булат";
    _ru["as_semen"]          = "Семён";
    _ru["as_artem"]          = "Артём";
    _ru["as_home"]           = "Общежитие";
    _ru["as_university"]     = "Университет";
    _ru["as_classroom"]      = "Аудитория";
    _ru["as_cafeteria"]      = "Столовая";
    _ru["as_street"]         = "Улица";
    _ru["as_exam"]           = "Экзамен";
    _ru["as_teacher"]        = "Преподаватель";
    _ru["as_rain"]           = "Дождь";
    _ru["as_sun"]            = "Солнце";
    _ru["as_success"]        = "УСПЕШНО!";
    _ru["as_fail"]           = "ПРОВАЛ!";
    _ru["as_well_done"]      = "Молодец!";
    _ru["as_failed"]         = "Неудача";
    _ru["as_unknown"]        = "Неизвестно";

    _ru["deb_imposter"]      = "Самозванец!";
    _ru["deb_burnout"]       = "Выгорание!";
    _ru["deb_broken_heart"]  = "<3 разбито!";
    _ru["deb_sleep"]         = "Сонный паралич!";
    _ru["deb_starving"]      = "Голод!";

    _ru["ctrl_title"]        = "УПРАВЛЕНИЕ";
    _ru["ctrl_text1"]        = "Игра использует цифровые меню.";
    _ru["ctrl_text2"]        = "Введите номер пункта и нажмите Enter.";
    _ru["ctrl_tips"]         = "Советы:";
    _ru["ctrl_tip1"]         = "Следите за уровнем голода и усталости";
    _ru["ctrl_tip2"]         = "Говорите с NPC -- они могут помочь";
    _ru["ctrl_tip3"]         = "Алла имеет романтическую сюжетную линию";
    _ru["ctrl_tip4"]         = "Покупайте цветы в магазине";
    _ru["ctrl_tip5"]         = "Готовьтесь к экзаменам!";
    _ru["ctrl_tip6"]         = "Следите за уровнем стресса";
    _ru["ctrl_endings"]      = "Концовки:";
    _ru["ctrl_end1"]         = "10 разных концовок в зависимости от выбора";
    _ru["ctrl_end2"]         = "Секретная романтическая: Алла отношения >= 80";
    _ru["ctrl_end3"]         = "Секретная армейская: особый выбор в концовке";

    _ru["set_title"]         = "НАСТРОЙКИ";
    _ru["set_current"]       = "Текущий профиль";
    _ru["set_compact"]       = "Компактный";
    _ru["set_wide"]          = "Широкий";
    _ru["set_changed"]       = "Профиль изменён";

    _ru["end_super"]         = "СУПЕР-КОНЦОВКА";
    _ru["end_good"]          = "ХОРОШАЯ КОНЦОВКА";
    _ru["end_normal"]        = "НОРМАЛЬНАЯ КОНЦОВКА";
    _ru["end_expelled"]      = "ОТЧИСЛЕНИЕ";
    _ru["end_academic"]      = "АКАДЕМИЧЕСКИЙ ОТПУСК";
    _ru["end_mental"]        = "ПСИХИАТРИЧЕСКАЯ БОЛЬНИЦА";
    _ru["end_starved"]       = "СМЕРТЬ ОТ ГОЛОДА";
    _ru["end_debtor"]        = "ВЕЧНЫЙ ДОЛЖНИК";
    _ru["end_romantic"]      = "СЕКРЕТНАЯ РОМАНТИЧЕСКАЯ КОНЦОВКА";
    _ru["end_army"]          = "СЕКРЕТНАЯ АРМЕЙСКАЯ КОНЦОВКА";
    _ru["end_unknown"]       = "НЕИЗВЕСТНО";

    _ru["end_super_body"]   = "Тимур сдал все экзамены на отлично!\nПрофессора поражены его знаниями.\nДрузья гордятся им.\nА Алла... Тимур и Алла по-настоящему влюблены.\n\nТимур получил красный диплом и предложение\nработы от лучшей IT-компании страны.\n\nСТУДЕНЧЕСКАЯ ЖИЗНЬ ЗАКОНЧЕНА. НАЧИНАЕТСЯ НАСТОЯЩАЯ!";
    _ru["end_good_body"]    = "Тимур успешно сдал сессию.\nНе всё прошло гладко, но он справился.\nВпереди лето, каникулы и новые планы.\n\nЖизнь продолжается!";
    _ru["end_normal_body"]  = "Сессия закончена. Были и взлёты, и падения.\nТимур выжил — и это главное.\nВпереди ещё много семестров...\n\nВ целом всё нормально.";
    _ru["end_expelled_body"]= "Тимура отчислили за академическую неуспеваемость.\nСлишком много долгов, слишком мало знаний.\n\nРодители разочарованы. Будущее туманно.\nНо это не конец света...\nХотя сейчас так и кажется.";
    _ru["end_academic_body"]= "Тимур взял академический отпуск.\nНервы сдали, сил больше нет.\n\nМожет быть, он вернётся через год.\nИли найдёт другой путь.\n\nГлавное — отдохнуть и восстановиться.";
    _ru["end_mental_body"]  = "Стресс сломал Тимура.\nБессонные ночи, постоянное напряжение...\n\nТеперь ему нужна профессиональная помощь.\nХорошая новость: экзамены здесь сдавать не надо.";
    _ru["end_starved_body"] = "Тимур так часто забывал поесть,\nчто организм не выдержал.\n\nЭкзамены теперь не важны.\nВажно было вовремя пообедать...";
    _ru["end_debtor_body"]  = "Тимур накопил столько долгов,\nчто никогда их не отдаст.\nСессия провалена, хвосты растут.\n\nЕщё один семестр...\nИ ещё один...\nИ ещё...";
    _ru["end_romantic_body"]= "Тимур и Алла — идеальная пара.\n\nВместе они сдали все экзамены.\nВместе строят планы на будущее.\n\nИ жили они долго и счастливо...\nНу, по крайней мере до следующей сессии.";
    _ru["end_army_body"]    = "Тимур решил, что учёба — не его.\nАрмия ждёт!\n\nДва года службы.\nНовые друзья, новые испытания.\n\nМожет быть, он вернётся доучиваться.\nА может, выберет другой путь.";

    // English
    _en["menu_new_game"]     = "New Game";
    _en["menu_load_game"]    = "Load Game";
    _en["menu_controls"]     = "Controls";
    _en["menu_settings"]     = "Settings";
    _en["menu_exit"]         = "Exit";
    _en["menu_your_choice"]  = "Your choice";
    _en["menu_restart"]      = "Start again";
    _en["menu_back"]         = "Back to menu";
    _en["menu_lang"]         = "Interface language";
    _en["menu_rus"]          = "Русский";
    _en["menu_eng"]          = "English";
    _en["menu_profile"]      = "Interface profile";
    _en["menu_compact"]      = "Compact (80 chars)";
    _en["menu_standard"]     = "Standard (120 chars)";
    _en["menu_wide"]         = "Wide (140 chars)";
    _en["menu_back_title"]   = "Back";
    _en["menu_title"]        = "STUDENT LIFE";
    _en["menu_welcome"]      = "Welcome to Ufa University of Science and Technology.";
    _en["menu_welcome2"]     = "First session ahead, new acquaintances, exams and choices";
    _en["menu_welcome3"]     = "that will affect the entire story.";
    _en["menu_current"]      = "Current profile";
    _en["menu_lang_current"] = "Language";

    _en["hud_day"]           = "Day";
    _en["hud_int"]           = "INT";
    _en["hud_en"]            = "EN";
    _en["hud_fat"]           = "FAT";
    _en["hud_hun"]           = "HUN";
    _en["hud_str"]           = "STR";
    _en["hud_money"]         = "RUB";

    _en["ui_press_enter"]    = "Press Enter...";
    _en["ui_your_choice"]    = "Your choice";
    _en["ui_prompt_choice"]  = "Your choice [%NUM%]:";
    _en["ui_saved"]          = "Game saved.";
    _en["ui_loaded"]         = "Save loaded.";
    _en["ui_invalid"]        = "Invalid input.";
    _en["ui_save_not_found"] = "Save not found.";

    _en["as_alla"]           = "Alla";
    _en["as_bulat"]          = "Bulat";
    _en["as_semen"]          = "Semen";
    _en["as_artem"]          = "Artem";
    _en["as_home"]           = "Home";
    _en["as_university"]     = "University";
    _en["as_classroom"]      = "Classroom";
    _en["as_cafeteria"]      = "Cafeteria";
    _en["as_street"]         = "Street";
    _en["as_exam"]           = "Exam";
    _en["as_teacher"]        = "Teacher";
    _en["as_rain"]           = "Rain";
    _en["as_sun"]            = "Sun";
    _en["as_success"]        = "SUCCESS!";
    _en["as_fail"]           = "FAIL!";
    _en["as_well_done"]      = "Well done!";
    _en["as_failed"]         = "Failed";
    _en["as_unknown"]        = "Unknown";

    _en["deb_imposter"]      = "Imposter!";
    _en["deb_burnout"]       = "Burnout!";
    _en["deb_broken_heart"]  = "<3 broken!";
    _en["deb_sleep"]         = "Sleep paralysis!";
    _en["deb_starving"]      = "Starving!";

    _en["ctrl_title"]        = "CONTROLS";
    _en["ctrl_text1"]        = "Game uses numeric menus.";
    _en["ctrl_text2"]        = "Enter the option number and press Enter.";
    _en["ctrl_tips"]         = "Tips:";
    _en["ctrl_tip1"]         = "Watch your hunger and fatigue levels";
    _en["ctrl_tip2"]         = "Talk to NPCs -- they can help";
    _en["ctrl_tip3"]         = "Alla has a romantic storyline";
    _en["ctrl_tip4"]         = "Buy flowers at the flower shop";
    _en["ctrl_tip5"]         = "Prepare for exams!";
    _en["ctrl_tip6"]         = "Watch your stress level";
    _en["ctrl_endings"]      = "Endings:";
    _en["ctrl_end1"]         = "10 different endings based on your choices";
    _en["ctrl_end2"]         = "Secret romantic: Alla relation >= 80";
    _en["ctrl_end3"]         = "Secret army: special choice in ending";

    _en["set_title"]         = "SETTINGS";
    _en["set_current"]       = "Current profile";
    _en["set_compact"]       = "Compact";
    _en["set_wide"]          = "Wide";
    _en["set_changed"]       = "Profile changed";

    _en["end_super"]         = "SUPER ENDING";
    _en["end_good"]          = "GOOD ENDING";
    _en["end_normal"]        = "NORMAL ENDING";
    _en["end_expelled"]      = "EXPELLED";
    _en["end_academic"]      = "ACADEMIC LEAVE";
    _en["end_mental"]        = "MENTAL HOSPITAL";
    _en["end_starved"]       = "STARVED TO DEATH";
    _en["end_debtor"]        = "ETERNAL DEBTOR";
    _en["end_romantic"]      = "SECRET ROMANTIC ENDING";
    _en["end_army"]          = "SECRET ARMY ENDING";
    _en["end_unknown"]       = "UNKNOWN";

    _en["end_super_body"]   = "Timur passed all exams with flying colors!\nProfessors are amazed by his knowledge.\nFriends are proud of him.\nAnd Alla... Timur and Alla are truly in love.\n\nTimur received a red diploma and a job offer\nfrom the best IT company in the country.\n\nSTUDENT LIFE IS OVER. REAL LIFE BEGINS!";
    _en["end_good_body"]    = "Timur successfully passed the session.\nNot everything went smoothly, but he managed.\nSummer, holidays and new plans await.\n\nLife goes on!";
    _en["end_normal_body"]  = "The session is over. It had its ups and downs.\nTimur survived and that's what matters.\nMany more semesters ahead...\n\nOverall, everything is fine.";
    _en["end_expelled_body"]= "Timur was expelled for academic failure.\nToo many debts, too little knowledge.\n\nParents are disappointed. Future is uncertain.\nBut this is not the end of the world...\nAlthough it feels like it right now.";
    _en["end_academic_body"]= "Timur took an academic leave.\nNerves gave out, no more strength left.\n\nMaybe he will return in a year.\nOr maybe he will find another path.\n\nThe main thing is to rest and recover.";
    _en["end_mental_body"]  = "Stress broke Timur.\nSleepless nights, constant tension...\n\nNow he needs professional help.\nGood news: no exams to take here.";
    _en["end_starved_body"] = "Timur forgot to eat so often\nthat his body couldn't take it anymore.\n\nExams don't matter now.\nWhat mattered was having lunch on time...";
    _en["end_debtor_body"]  = "Timur accumulated so many debts\nthat he can never repay them.\nSession failed, tails keep growing.\n\nOne more semester...\nAnd another one...\nAnd another...";
    _en["end_romantic_body"]= "Timur and Alla -- the perfect couple.\n\nTogether they passed all exams.\nTogether they make plans for the future.\n\nAnd they lived happily ever after...\nWell, at least until next session.";
    _en["end_army_body"]    = "Timur decided that studying is not for him.\nThe army awaits!\n\nTwo years of service.\nNew friends, new challenges.\n\nMaybe he'll come back to finish his studies.\nOr maybe he'll choose a different path.";
}

// ---- Language manager implementation ----

void Lang::set(Language lang) {
    ensureInit();
    _lang = lang;
}

Lang::Language Lang::current() {
    ensureInit();
    return _lang;
}

std::string Lang::currentName() {
    ensureInit();
    return _lang == Russian ? "Русский" : "English";
}

std::string Lang::currentNameEn() {
    ensureInit();
    return _lang == Russian ? "Russian" : "English";
}

const std::string& Lang::get(const std::string& key) {
    ensureInit();
    if (_lang == Russian) {
        auto it = _ru.find(key);
        if (it != _ru.end()) return it->second;
    } else {
        auto it = _en.find(key);
        if (it != _en.end()) return it->second;
    }
    static const std::string empty;
    auto it = _ru.find(key);
    if (it != _ru.end()) return it->second;
    return empty;
}

Lang::Language& Lang::state() {
    ensureInit();
    return _lang;
}

std::map<std::string, std::string>& Lang::ru() { ensureInit(); return _ru; }
std::map<std::string, std::string>& Lang::en() { ensureInit(); return _en; }
void Lang::initRu() { ensureInit(); }
void Lang::initEn() { ensureInit(); }
