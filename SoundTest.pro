QT += core multimedia
QT -= gui

CONFIG += c++20 console

TARGET = SoundTest

SOURCES += main.cpp \
    Sound.cpp \
    SoundDevice.cpp

HEADERS += \
    Sound.h \
    SoundDevice.h
