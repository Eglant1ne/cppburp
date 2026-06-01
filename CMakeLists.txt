cmake_minimum_required(VERSION 3.16)
project(ProxyLab VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Try Qt6 first, fall back to Qt5.15 ──────────────────────────────────────
find_package(Qt6 COMPONENTS Core Gui Widgets Network Sql QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Network Sql REQUIRED)
endif()

set(SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    httpparser.h
    httpparser.cpp
    trafficrecord.h
    proxyconnection.h
    proxyconnection.cpp
    proxyserver.h
    proxyserver.cpp
    repeaterclient.h
    repeaterclient.cpp
    storagemanager.h
    storagemanager.cpp
)

add_executable(ProxyLab ${SOURCES})

# ── Force UTF-8 source encoding on MSVC ─────────────────────────────────────
if(MSVC)
    target_compile_options(ProxyLab PRIVATE /utf-8)
endif()

if(Qt6_FOUND)
    target_link_libraries(ProxyLab PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Sql)
else()
    target_link_libraries(ProxyLab PRIVATE
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Sql)
endif()

if(WIN32)
    set_target_properties(ProxyLab PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# ── Documentation: run .\GenerateDocs.ps1 (Windows) or doxygen Doxyfile (any OS)
# Doxygen target intentionally excluded from MSBuild — use the script instead.
