win32:DEFINES += _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS
CONFIG += c++17
CONFIG -= app_bundle

QT += widgets network testlib

win32:LIBS += -lwindowsapp

INCLUDEPATH += $$PWD/../external $$PWD/../src/service $$PWD/../src $$PWD/../src/capture

HEADERS += \
  ../src/languagecodes.h \
  ../src/service/updates.h \
  ../src/service/widgetstate.h \
  ../src/task.h \
  mocknetworkaccessmanager.h \
  ../src/ocr/winocr.h

SOURCES += \
  ../external/gtest/gtest-all.cc \
  ../src/languagecodes.cpp \
  ../src/service/geometryutils.cpp \
  ../src/service/updates.cpp \
  ../src/service/widgetstate.cpp \
  ../src/service/debug.cpp \
  ../external/miniz/miniz.c \
  ../src/capture/capturearea.cpp \
  geometryutils_test.cpp \
  languagecodes_test.cpp \
  main.cpp \
  updates_test.cpp \
  widgetstate_test.cpp \
  mocknetworkaccessmanager.cpp \
  translation_pipeline_test.cpp \
  capturearea_test.cpp \
  ../src/ocr/winocr.cpp \
  winocr_test.cpp
