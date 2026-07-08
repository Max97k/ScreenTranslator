CONFIG += c++17
CONFIG -= app_bundle

win32:DEFINES += _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS

QT += widgets network testlib

INCLUDEPATH += $$PWD/../external $$PWD/../src/service $$PWD/../src $$PWD/../src/capture $$PWD/../src/ocr $$PWD/../src/translate

HEADERS += \
  ../src/languagecodes.h \
  ../src/service/updates.h \
  ../src/service/widgetstate.h \
  ../src/task.h \
  ../src/commonmodels.h \
  mocknetworkaccessmanager.h

SOURCES += \
  ../external/gtest/gtest-all.cc \
  ../src/commonmodels.cpp \
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
  commonmodels_test.cpp \
  mock_commonmodels_deps.cpp
