QT += core multimedia
QT -= gui

CONFIG += c++14

TARGET = SoundTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    EncodedSound.cpp \
    SoundDecoder.cpp \
    SoundDevice.cpp

HEADERS += \
    Sound.h \
    EncodedSound.h \
    SoundDecoder.h \
    SoundDevice.h
