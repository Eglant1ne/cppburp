# ProxyLab — Qt HTTP Interception Suite

Минималистичный Burp Suite-подобный интерфейс на **Qt 5/6 + C++17** для Windows.
Бело-синяя цветовая схема (Material Blue).

---

## Структура файлов

```
ProxyLab/
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── ProxyLab.pro      ← qmake
└── CMakeLists.txt    ← cmake
```

---

## Вкладки

| Вкладка     | Описание |
|-------------|----------|
| **Proxy**   | Таблица истории запросов, просмотр Request/Response, кнопки Forward / Drop / Send to Repeater, включение перехвата |
| **Repeater**| Редактор запроса с ручной отправкой, просмотр ответа |
| **Scanner** | Таблица уязвимостей (SQL Injection, XSS и др.) + панель Advisory |
| **HTTP Log**| Живой лог всего трафика с цветовой подсветкой статусов |

---

## Сборка

### Вариант 1 — qmake (Qt Creator / CLI)

```bat
cd ProxyLab
qmake ProxyLab.pro
nmake          # или mingw32-make / jom
```

Или просто откройте `ProxyLab.pro` в **Qt Creator** и нажмите Build.

### Вариант 2 — CMake

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

---

## Требования

- Qt 5.15+ или Qt 6.x
- C++17-совместимый компилятор (MSVC 2019+, MinGW 11+)
- Только модули `Core`, `Gui`, `Widgets` — никаких сторонних зависимостей