QT       += testlib
QT       -= gui

TARGET = tst_accesscachetest
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_accesscachetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
