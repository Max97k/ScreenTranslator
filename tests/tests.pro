CONFIG += c++17
CONFIG -= app_bundle

QT += widgets network testlib

INCLUDEPATH += $$PWD/../external $$PWD/../src/service $$PWD/../src $$PWD/../src/capture

HEADERS += \
  ../src/languagecodes.h \
  ../src/service/updates.h \
  ../src/service/widgetstate.h \
  ../src/service/runatsystemstart.h \
  ../src/task.h \
  ../src/settingsvalidator.h \
  ../src/settings.h \
  ../src/commonmodels.h \
  mocknetworkaccessmanager.h

SOURCES += \
  ../external/gtest/gtest-all.cc \
  ../src/languagecodes.cpp \
  ../src/service/geometryutils.cpp \
  ../src/service/updates.cpp \
  ../src/service/widgetstate.cpp \
  ../src/service/runatsystemstart.cpp \
  ../src/service/debug.cpp \
  ../src/settings.cpp \
  ../src/settingsvalidator.cpp \
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
  mock_commonmodels.cpp \
  settingsvalidator_test.cpp
