# ProxyLab v2 — HTTP Interception Suite

**ProxyLab** — это инструмент для перехвата, анализа и повторной отправки HTTP/HTTPS трафика,
написанный на **Qt 5/6 + C++17**. По функциональности близок к Burp Suite Community Edition:
перехват запросов, редактирование на лету, Repeater, журнал трафика, коллекции запросов с
хранением в SQLite.

---

## Содержание

1. [Возможности](#возможности)
2. [Требования](#требования)
3. [Установка и сборка](#установка-и-сборка)
4. [Настройка браузера](#настройка-браузера)
5. [Использование](#использование)
6. [Архитектура](#архитектура)
7. [Документация API](#документация-api)
8. [Структура файлов](#структура-файлов)

---

## Возможности

| Модуль         | Описание |
|----------------|----------|
| **Proxy**      | Перехват HTTP-запросов на порту 8080, редактирование и пересылка/отклонение |
| **Intercept**  | Очередь перехваченных запросов, поддержка одновременных соединений |
| **Repeater**   | Прямая отправка произвольных HTTP/HTTPS запросов с авто-нормализацией |
| **HTTP Log**   | Полная история трафика с инспектором запроса/ответа |
| **Collections**| Сохранение запросов в именованные группы, SQLite-персистентность |
| **CONNECT**    | Прозрачный TLS-туннель для HTTPS-трафика |

---

## Требования

- **Qt** 5.15+ или Qt 6.x
- Модули Qt: `Core`, `Gui`, `Widgets`, `Network`, `Sql`
- **CMake** 3.16+
- **C++17**: MSVC 2019+, MinGW 11+, GCC 9+, Clang 10+
- SQLite встроен в Qt — внешних зависимостей нет

---

## Установка и сборка

### Вариант 1 — CMake (рекомендуется)

```bash
# Клонировать репозиторий
git clone https://github.com/your-org/ProxyLab.git
cd ProxyLab

# Настроить сборку (укажите путь к Qt через CMAKE_PREFIX_PATH)
cmake -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.5.3/gcc_64"

# Собрать
cmake --build build --config Release

# Запустить
./build/ProxyLab          # Linux/macOS
build\Release\ProxyLab.exe  # Windows
```

**Windows + MSVC пример:**
```bat
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2019_64"
cmake --build build --config Release
```

### Вариант 2 — qmake

```bash
cd ProxyLab
qmake ProxyLab.pro
make          # Linux/macOS
nmake         # Windows MSVC
```

Или откройте `ProxyLab.pro` в **Qt Creator** и нажмите **Build**.

### Сборка документации (опционально)

Если установлен Doxygen:
```bash
cmake --build build --target ProxyLabDocs
# HTML-документация → docs/html/index.html
```

---

## Настройка браузера

Укажите HTTP-прокси: **127.0.0.1 : 8080**

### Firefox

1. Настройки → Основные → Настройки сети → Настроить
2. Выбрать «Ручная настройка прокси»
3. HTTP Прокси: `127.0.0.1`, Порт: `8080`
4. Установить галку «Использовать этот прокси для HTTPS»

### Chrome / Edge (расширение FoxyProxy)

1. Установить расширение **FoxyProxy Standard**
2. Добавить прокси: тип HTTP, хост `127.0.0.1`, порт `8080`
3. Активировать для нужных сайтов

### Системный прокси (Windows)

Параметры → Сеть и Интернет → Прокси → Использовать прокси-сервер:
`127.0.0.1:8080`

> **HTTPS**: ProxyLab поддерживает метод `CONNECT` для прозрачного TLS-туннелирования.
> MITM-сертификат не используется — содержимое зашифрованного трафика не видно.

---

## Использование

### Вкладка Proxy — перехват запросов

1. Запустите ProxyLab, браузер настроен на прокси.
2. Поставьте галку **Intercept is ON**.
3. Выполните запрос в браузере — он появится в панели «Request».
4. Отредактируйте запрос при необходимости.
5. Нажмите **▶ Forward** — запрос уйдёт на сервер, ответ появится в «Response».
   Или **✖ Drop** — браузер получит 403.
6. Кнопка **↗ Send to Repeater** скопирует запрос на вкладку Repeater.

**Счётчик Queue** показывает, сколько запросов ждёт в очереди пока вы работаете с текущим.

### Вкладка Repeater — повторная отправка

```
Target:  api.example.com:443   [x] HTTPS
```

1. Вставьте или отредактируйте raw HTTP-запрос в левой панели.
2. Укажите хост и порт в поле Target.
3. Нажмите **▶ Send** — ответ появится справа.
4. Кнопка **💾 Save…** сохраняет запрос в коллекцию.

ProxyLab автоматически нормализует:
- окончания строк (`\n` → `\r\n`)
- `Connection: close`
- `Content-Length` (пересчёт после редактирования тела)

### Вкладка HTTP Log — история

Таблица содержит все завершённые обмены сессии. Клик по строке показывает
сырые байты запроса и ответа в инспекторе снизу. Правый клик даёт меню:
«Send to Repeater» и «Save to Collection».

### Collections — коллекции запросов

Меню **Project → New Collection Group…** создаёт именованную папку.
Меню **Project → Save Request to Group…** (или кнопка 💾 в Repeater) сохраняет
текущий запрос. В доке «Collections» слева клик по запросу загружает его в Repeater.
Правый клик → Delete для удаления.

### Смена порта прокси

Меню **Tools → Change Proxy Port…** — новое значение вступает в силу немедленно.

---

## Архитектура

```
main.cpp
  └── MainWindow              ← UI-контроллер (Qt Widgets)
        ├── ProxyServer       ← QTcpServer, порт 8080
        │     └── ProxyConnection × N  ← одно TCP-соединение
        │           ├── HttpParser     ← парсер запросов
        │           └── QSslSocket     ← TLS для HTTPS
        ├── RepeaterClient    ← изолированный HTTP-клиент
        └── StorageManager    ← SQLite (QtSql)
```

**Поток данных (перехват):**

```
Browser → ProxyConnection.onClientData()
        → HttpParser.parse()
        → emit intercepted(id, raw)   ← если intercept ON
        → MainWindow.onRequestIntercepted()
        → [пользователь редактирует]
        → ProxyConnection.forwardRequest()
        → connectToTarget() → QTcpSocket/QSslSocket
        → Origin Server → response
        → emit finished(record)
        → StorageManager.saveRecord()
```

### Ключевые классы

| Класс | Файл | Назначение |
|-------|------|------------|
| `HttpParser` | `httpparser.h/.cpp` | Парсинг HTTP/1.x запросов, разбор host:port |
| `ProxyConnection` | `proxyconnection.h/.cpp` | Одно соединение клиент↔сервер, туннель CONNECT |
| `ProxyServer` | `proxyserver.h/.cpp` | TCP-сервер, очередь перехвата |
| `RepeaterClient` | `repeaterclient.h/.cpp` | Прямой HTTP/HTTPS клиент с таймаутом |
| `StorageManager` | `storagemanager.h/.cpp` | SQLite CRUD для истории и коллекций |
| `TrafficRecord` | `trafficrecord.h` | Структура данных одного HTTP-обмена |

---

## Документация API

HTML-документация генерируется Doxygen:

```bash
cmake --build build --target ProxyLabDocs
# Открыть docs/html/index.html
```

Все публичные классы, методы, параметры и возвращаемые значения задокументированы
в заголовочных файлах в формате Doxygen (`@brief`, `@param`, `@return`, `@throws`).

---

## Структура файлов

```
ProxyLab/
├── CMakeLists.txt           ← основной cmake (Qt5/Qt6, Doxygen)
├── ProxyLab.pro             ← qmake (альтернатива)
├── .clang-format            ← стиль форматирования (Google Style, indent=4)
├── README.md                ← это руководство
├── main.cpp                 ← точка входа, try/catch верхнего уровня
├── trafficrecord.h          ← POD-структура TrafficRecord
├── httpparser.h/.cpp        ← HTTP парсер
├── proxyconnection.h/.cpp   ← одно проксируемое соединение
├── proxyserver.h/.cpp       ← TCP сервер + очередь перехвата
├── repeaterclient.h/.cpp    ← Repeater HTTP клиент
├── storagemanager.h/.cpp    ← SQLite персистентность
├── mainwindow.h/.cpp        ← главное окно Qt Widgets
└── docs/                    ← сгенерированная Doxygen документация
```

---

## База данных

Файл `proxylab.sqlite` создаётся автоматически:

- **Windows**: `%APPDATA%\ProxyLab\ProxyLab\proxylab.sqlite`
- **Linux**: `~/.local/share/ProxyLab/ProxyLab/proxylab.sqlite`

Схема:

| Таблица | Содержимое |
|---------|------------|
| `traffic` | История всех HTTP-обменов |
| `groups` | Коллекции (папки) |
| `saved_requests` | Сохранённые запросы в коллекциях |

---

## Обработка ошибок

- Все ошибки сети передаются через сигналы Qt и отображаются в статус-баре.
- Ошибки открытия БД и некорректные HTTP-запросы генерируют `std::exception` и
  перехватываются в `main()` / соответствующих слотах.
- Таймаут Repeater: **20 секунд** (возвращает частичный ответ если данные уже получены).
