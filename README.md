# SoundTest
A small app for testing QSoundEffect and QAudioOutput classes

# Building
Release: qmake ../SoundTest/SoundTest.pro -r -spec linux-g++ QMAKE_CXX=g++-5
Debug:   qmake ../SoundTest/SoundTest.pro -r -spec linux-g++ CONFIG+=debug CONFIG+=qml_debug QMAKE_CXX=g++-5

Usage: ./SoundTest [options] source
Sound Test

Options:
  -h, --help     Displays this help.
  -v, --version  Displays version information.
  -b             Reproduce QSoundEffect bug.

Arguments:
  source         Sound file to play.

