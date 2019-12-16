QT += testlib

CONFIG += qt console warn_on depend_includepath testcase c++17
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/../../app

SOURCES += \
    ../../app/logicalclocks.cpp \
    tst_logicalclocks.cpp

HEADERS += \
    ../../app/logicalclocks.h
