# Будни студента

**Текстовая приключенческая игра.**  
Вы — студент. Впереди 8 дней, экзамены, отношения, привычки и случайные события.  
Сможете ли вы дожить до конца сессии?

---

## Вариант 1: скачать готовую сборку

1. Перейдите на страницу **Releases** этого репозитория.
2. Скачайте последний архив `student_life_v*.zip`.
3. Распакуйте и запустите `student_life.exe`.

Ничего устанавливать не нужно.

---

## Вариант 2: собрать из исходников

### Требования

- **Windows** (10 или 11, 64-bit)
- **Microsoft Visual C++** (компилятор `cl.exe`):

  | Что установить | Где взять |
  |---|---|
  | Visual Studio 2022 Community | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/vs/) |
  | Visual Studio Build Tools | [скачать](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022) |

  При установке обязательно выберите компонент **"Разработка классических приложений на C++"**.

### Сборка

1. Откройте **Командную строку** (cmd) или PowerShell.
2. Перейдите в папку с проектом:
   ```cmd
   cd путь\к\StudentLife
   ```
3. Запустите:
   ```cmd
   build.bat
   ```

Скрипт сам найдёт установленную Visual Studio через `vswhere.exe` и соберёт `student_life.exe`.

### Запуск

```cmd
student_life.exe
```

---

## Если нет компилятора

- Скачайте готовый `.exe` из **Releases** — он не требует установки.
- Или установите **Visual Studio Build Tools** (бесплатно, ~1 ГБ):
  https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022

---

## Структура проекта

```
StudentLife/
├── build.bat            # Сборщик проекта
├── student_life.exe     # Готовая игра (после сборки)
├── main.cpp             # Точка входа
├── core/                # Игровая логика
├── player/              # Игрок и характеристики
├── npc/                 # NPC
├── data/                # Константы, строки, ASCII-арт
├── exams/               # Система экзаменов
├── events/              # Случайные события
├── systems/             # Системы (голод, отношения, привычки, ...)
├── ui/                  # Консольный интерфейс
├── scenes/              # Сцены
├── save/                # Сохранение
└── .gitignore
```

---

## Лицензия

MIT
