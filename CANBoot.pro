#-------------------------------------------------
#
# Project created by QtCreator 2018-02-10T13:33:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CANBoot
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        canboot_library/canboot.cpp \
        console.cpp \
        canboot_library/bootfirmware.cpp \
    filedialog/filefialog.cpp

HEADERS += \
        mainwindow.h \
        canboot_library/canboot.h \
        chai_library/chai.h \
        console.h \
        canboot_library/bootfirmware.h \
    filedialog/filedialog.h

INCLUDEPATH += "$$PWD/chai_library"

LIBS += -L"$$PWD/chai_library" -lchai

FORMS += \
    mainwindow.ui

DISTFILES += \
    chai_library/chai.lib \
    chai_library/chai.dll \
    canboot_library/firmware.bin
