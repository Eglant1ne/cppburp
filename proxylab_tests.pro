QT += core network
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = proxylab_tests

SOURCES += \
    test_main.cpp \
    httpparser.cpp

HEADERS += \
    httpparser.h \
    trafficrecord.h
