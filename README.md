# ProxyLab v2 — Qt HTTP Interception Suite

Burp Suite-подобный инструмент на **Qt 5/6 + C++17** для Windows/Linux.
Белo-синяя цветовая схема (Material Blue).

---
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\ПОЛНЫЙ_ПУТЬ_К_ПАПКЕ_QT\6.5.3\msvc2019_64"

## Что реализовано (5 этапов)

| Этап | Что сделано |
|------|-------------|
| **1 — Сетевое ядро** | `ProxyServer` (QTcpServer, порт 8080), `ProxyConnection` (клиент↔сервер TCP/TLS), `HttpParser` (метод, хост, заголовки, CONNECT) |
| **2 — Intercept Engine** | Блокировка потока при `interceptOn=true`, сигнал `requestIntercepted` → UI, слоты Forward / Drop |
| **3 — История + Инспектор** | `TrafficRecord`, история в `m_historyTable` и `m_logTable`, клик по строке → `m_logInspectReq/Resp` |
| **4 — Repeater** | `RepeaterClient` (прямое TCP/TLS соединение, минуя прокси), авто-подстановка хоста, отображение raw-ответа |
| **5 — Collections & Storage** | `StorageManager` (SQLite через QtSql), группы запросов, CRUD, `QDockWidget` с `QTreeWidget` слева |

---

## Структура файлов

```
ProxyLab_v2/
├── main.cpp
├── mainwindow.h / .cpp      ← UI + главный контроллер
├── trafficrecord.h          ← структура данных трафика
├── httpparser.h / .cpp      ← лёгкий HTTP-парсер
├── proxyconnection.h / .cpp ← одно соединение клиент↔сервер
├── proxyserver.h / .cpp     ← TCP-слушатель 127.0.0.1:8080
├── repeaterclient.h / .cpp  ← изолированный HTTP-клиент
├── storagemanager.h / .cpp  ← SQLite: история + коллекции
├── ProxyLab.pro             ← qmake
└── CMakeLists.txt           ← cmake
```

---

## Сборка

### Вариант 1 — qmake (Qt Creator / CLI)

```bat
cd ProxyLab_v2
qmake ProxyLab.pro
nmake          # или mingw32-make / jom
```

Или откройте `ProxyLab.pro` в **Qt Creator** и нажмите Build.

### Вариант 2 — CMake

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

---

## Требования

- Qt 5.15+ или Qt 6.x
- Модули: `Core`, `Gui`, `Widgets`, **`Network`**, **`Sql`**
- C++17 (MSVC 2019+, MinGW 11+, GCC 9+, Clang 10+)
- SQLite встроен в Qt — дополнительных зависимостей нет

---

## Настройка браузера (FoxyProxy / системный прокси)

Укажите HTTP-прокси: `127.0.0.1 : 8080`

Для HTTPS поддерживается `CONNECT`-туннель (прозрачный, без MITM-сертификата).

---

## База данных

Файл `proxylab.sqlite` сохраняется в:
- **Windows**: `%APPDATA%\ProxyLab\ProxyLab\proxylab.sqlite`
- **Linux**: `~/.local/share/ProxyLab/ProxyLab/proxylab.sqlite`

История трафика и коллекции сохраняются между запусками.

---

## Архитектурные заметки

- **ProxyServer** создаёт `ProxyConnection` на каждое входящее TCP-соединение.
- **ProxyConnection** работает в том же потоке (event loop), буферизует данные и либо туннелирует их (CONNECT), либо ждёт действия пользователя (intercept=true).
- **RepeaterClient** полностью изолирован от прокси — устанавливает прямое соединение по хосту из поля Target.
- **StorageManager** использует именованный `QSqlDatabase` (`proxylab_conn`), чтобы не конфликтовать с другими соединениями в приложении.
