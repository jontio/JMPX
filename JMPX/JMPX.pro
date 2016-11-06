# -------------------------------------------------
# Project created by QtCreator 2015-09-24T05:34:57
# -------------------------------------------------

QT       += core gui

#needed for MPRIS2 (now playing class on linux)
unix {
QT       += dbus
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JMPX
TEMPLATE = app

RESOURCES += \
    jmpx.qrc

FORMS += \
    mainwindow.ui \
    options.ui

DISTFILES += \
    myappico.ico \
    LICENSE \
    myapp.rc \
    ../README.md \
    images/screenshot-linux.png \
    images/screenshot-win.png

RC_FILE = myapp.rc

HEADERS += \
    mainwindow.h \
    vol/volumemeter.h \
    ../libJMPX/JMPXInterface.h \
    options.h \
    fileloader.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    vol/volumemeter.cpp \
    options.cpp \
    fileloader.cpp

unix {
HEADERS += nowplaying_linux.h
SOURCES += nowplaying_linux.cpp
}

win32 {
HEADERS += nowplaying_windows.h
SOURCES += nowplaying_windows.cpp
}

macx {
HEADERS += nowplaying_mac.h
SOURCES += nowplaying_mac.cpp
}

