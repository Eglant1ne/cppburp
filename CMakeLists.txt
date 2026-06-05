cmake_minimum_required(VERSION 3.16)

project(ProxyLab VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Qt ─────────────────────────────────────────────────────
find_package(Qt6 COMPONENTS Core Gui Widgets Network Sql Test QUIET)

if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Network Sql Test REQUIRED)
endif()

# ── Main application sources ──────────────────────────────
set(SOURCES
    src/main.cpp

    src/mainwindow.h
    src/mainwindow.cpp

    src/httpparser.h
    src/httpparser.cpp

    src/trafficrecord.h

    src/proxyconnection.h
    src/proxyconnection.cpp

    src/proxyserver.h
    src/proxyserver.cpp

    src/repeaterclient.h
    src/repeaterclient.cpp

    src/storagemanager.h
    src/storagemanager.cpp
)

# ── Main executable ───────────────────────────────────────
add_executable(ProxyLab ${SOURCES})

target_include_directories(ProxyLab PRIVATE src)

# ── UTF-8 for MSVC ────────────────────────────────────────
if(MSVC)
    target_compile_options(ProxyLab PRIVATE /utf-8)
endif()

# ── Link Qt ───────────────────────────────────────────────
if(Qt6_FOUND)
    target_link_libraries(ProxyLab PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
    )
else()
    target_link_libraries(ProxyLab PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Network
        Qt5::Sql
    )
endif()

# ── Windows GUI app ───────────────────────────────────────
if(WIN32)
    set_target_properties(ProxyLab PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# ── Tests ─────────────────────────────────────────────────
enable_testing()

add_subdirectory(tests)