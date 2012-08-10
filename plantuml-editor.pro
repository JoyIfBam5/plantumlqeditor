QT += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = plant-uml-editor

TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    previewwidget.cpp \
    preferencesdialog.cpp

HEADERS  += mainwindow.h \
    previewwidget.h \
    preferencesdialog.h

FORMS += \
    preferencesdialog.ui
