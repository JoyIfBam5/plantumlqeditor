QT       += testlib
QT       -= gui

TARGET = tst_assistantxmlreadertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_assistantxmlreadertest.cpp \
    ../../assistantxmlreader.cpp

HEADERS += \
    ../../assistantxmlreader.h

INCLUDEPATH += ../..

DEFINES += SRCDIR=\\\"$$PWD/\\\"
