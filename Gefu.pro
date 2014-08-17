#-------------------------------------------------
#
# Project created by QtCreator 2014-08-14T11:25:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Gefu
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    folderpanel.cpp \
    renamesingledialog.cpp \
    renamemultidialog.cpp \
    operationdialog.cpp \
    irenamedialog.cpp \
    iworker.cpp \
    renameworker.cpp \
    deleteworker.cpp

HEADERS  += mainwindow.h \
    folderpanel.h \
    singleton.hpp \
    renamesingledialog.h \
    renamemultidialog.h \
    operationdialog.h \
    common.h \
    irenamedialog.h \
    iworker.h \
    renameworker.h \
    deleteworker.h

FORMS    += mainwindow.ui \
    folderpanel.ui \
    renamesingledialog.ui \
    renamemultidialog.ui \
    operationdialog.ui

RESOURCES += \
    resource.qrc
