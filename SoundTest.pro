QT += core multimedia
QT -= gui

CONFIG += c++14

TARGET = SoundTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    Sound.cpp \
    SoundDevice.cpp

HEADERS += \
    Sound.h \
    SoundDevice.h
