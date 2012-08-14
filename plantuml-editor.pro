QT += core gui svg

QMAKE_CXXFLAGS += -std=c++0x

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = plant-uml-editor

TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    previewwidget.cpp \
    preferencesdialog.cpp \
    assistantxmlreader.cpp \
    filecache.cpp \
    utils.cpp

HEADERS  += mainwindow.h \
    previewwidget.h \
    preferencesdialog.h \
    assistantxmlreader.h \
    filecache.h \
    settingsconstants.h \
    utils.h

FORMS += \
    preferencesdialog.ui

OTHER_FILES += \
    assistant.xml
