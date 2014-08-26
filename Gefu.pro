#-------------------------------------------------
#
# Project created by QtCreator 2014-08-14T11:25:36
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = Gefu
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
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
    history.cpp \
    historydialog.cpp \
    preferencedialog.cpp \
    colorsamplemodel.cpp \
    simpletextview.cpp \
    searchbox.cpp \
    folderview.cpp \
    locationbox.cpp \
    simpleimageview.cpp

HEADERS  += mainwindow.h \
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
    history.h \
    historydialog.h \
    preferencedialog.h \
    colorsamplemodel.h \
    simpletextview.h \
    searchbox.h \
    folderview.h \
    locationbox.h \
    version.h \
    simpleimageview.h

FORMS    += mainwindow.ui \
    renamesingledialog.ui \
    renamemultidialog.ui \
    operationdialog.ui \
    overwritedialog.ui \
    sortdialog.ui \
    historydialog.ui \
    preferencedialog.ui

RESOURCES += \
    resource.qrc

OTHER_FILES += \
    Gefu.rc \
    README.md \
    ReadMe.txt

macx {
    ICON = ./images/Gefu.icns
}

win32 {
    RC_FILE = Gefu.rc
}
