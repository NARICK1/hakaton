#pragma once
#include <string>

// Режим разработчика — отдельный демонстрационный раздел
class DevMode {
public:
    // Главное меню режима разработчика
    static void ShowDevMenu();

    // 1. Демо анимаций
    static void DemoAnimations();

    // 2. Галерея персонажей
    static void GalleryCharacters();

    // 3. Галерея локаций
    static void GalleryLocations();

    // 4. Комикс (кат-сцены)
    static void ShowComic();

    // 5. Демонстрация интерфейсов (3 режима)
    static void DemoInterfaces();

    // 6. Галерея концовок
    static void GalleryEndings();

    // 7. Симуляция памяти NPC
    static void SimulateNPCMemory();

    // 8. Симуляция отношений
    static void SimulateRelations();

    // 9. Демонстрация событий
    static void DemoEvents();

    // 10. Бенчмарк / отладочная сводка
    static void Benchmark();

private:
    // Helper: показать подзаголовок
    static void showSubheader(const std::string& title);
    // Helper: ожидание Enter
    static void waitEnter();
    // Helper: демо-анимация глитча
    static void glitchEffect(int lines = 3);
    // Helper: демо-загрузка
    static void loadingDemo();
};
