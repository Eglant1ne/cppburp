QT       += core gui widgets network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++17
TARGET    = ProxyLab
TEMPLATE  = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    httpparser.cpp \
    proxyconnection.cpp \
    proxyserver.cpp \
    repeaterclient.cpp \
    storagemanager.cpp

HEADERS += \
    mainwindow.h \
    httpparser.h \
    trafficrecord.h \
    proxyconnection.h \
    proxyserver.h \
    repeaterclient.h \
    storagemanager.h

# Windows: hide console window in release
win32:CONFIG(release, debug|release): QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS
