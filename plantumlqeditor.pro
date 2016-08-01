QT += core gui svg

QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = plantumlqeditor

TEMPLATE = app

include(thirdparty/qtsingleapplication/src/qtsingleapplication.pri)

SOURCES += \
    textedit.cpp \
    main.cpp\
    mainwindow.cpp \
    previewwidget.cpp \
    preferencesdialog.cpp \
    assistantxmlreader.cpp \
    filecache.cpp \
    utils.cpp \
    recentdocuments.cpp

HEADERS += \
    textedit.h \
    mainwindow.h \
    previewwidget.h \
    preferencesdialog.h \
    assistantxmlreader.h \
    filecache.h \
    settingsconstants.h \
    utils.h \
    recentdocuments.h

FORMS += \
    preferencesdialog.ui

OTHER_FILES += \
    assistant.svg \
    assistant.xml \
    plantumlqeditor-mime.xml \
    plantumlqeditor.desktop \
    plantumlqeditor-singleinstance.desktop \
    README.mime \
    README \
    AUTHORS \
    COPYING \
    icon.svg \
    icon*.png \
    docs/* \
    icons/*

RESOURCES += \
    plantumlqeditor.qrc
