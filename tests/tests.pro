CONFIG += c++17
CONFIG -= app_bundle

QT += widgets network testlib

INCLUDEPATH += $$PWD/../external $$PWD/../src/service $$PWD/../src

HEADERS += \
  ../src/service/updates.h \
  ../src/task.h \
  ../src/languagecodes.h \
  mocknetworkaccessmanager.h

SOURCES += \
  ../external/gtest/gtest-all.cc \
  ../src/service/geometryutils.cpp \
  ../src/service/updates.cpp \
  ../src/service/debug.cpp \
  ../src/languagecodes.cpp \
  ../external/miniz/miniz.c \
  geometryutils_test.cpp \
  languagecodes_test.cpp \
  main.cpp \
  updates_test.cpp \
  mocknetworkaccessmanager.cpp \
  translation_pipeline_test.cpp
