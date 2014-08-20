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
    deleteworker.cpp \
    overwritedialog.cpp \
    copymoveworker.cpp \
    sortdialog.cpp \
    filetablemodel.cpp \
    filetableview.cpp

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
    deleteworker.h \
    overwritedialog.h \
    copymoveworker.h \
    sortdialog.h \
    filetablemodel.h \
    filetableview.h

FORMS    += mainwindow.ui \
    folderpanel.ui \
    renamesingledialog.ui \
    renamemultidialog.ui \
    operationdialog.ui \
    overwritedialog.ui \
    sortdialog.ui

RESOURCES += \
    resource.qrc

OTHER_FILES += \
    Gefu.rc \
    README.md

mac {
    ICON = ./images/Gefu.icns
}

win32 {
    RC_FILE = Gefu.rc
}
