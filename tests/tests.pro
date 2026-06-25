CONFIG += c++17
CONFIG -= app_bundle

QT += widgets network testlib

INCLUDEPATH += $$PWD/../external $$PWD/../src/service $$PWD/../src $$PWD/../src/capture

HEADERS += \
  ../src/service/updates.h \
  ../src/task.h \
  mocknetworkaccessmanager.h

SOURCES += \
  ../external/gtest/gtest-all.cc \
  ../src/service/geometryutils.cpp \
  ../src/service/updates.cpp \
  ../src/service/debug.cpp \
  ../external/miniz/miniz.c \
  ../src/capture/capturearea.cpp \
  geometryutils_test.cpp \
  main.cpp \
  updates_test.cpp \
  mocknetworkaccessmanager.cpp \
  translation_pipeline_test.cpp \
  capturearea_test.cpp
