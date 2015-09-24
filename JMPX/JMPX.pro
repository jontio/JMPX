# -------------------------------------------------
# Project created by QtCreator 2015-09-24T05:34:57
# -------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JMPX
TEMPLATE = app

RESOURCES += \
    icons.qrc

FORMS += \
    mainwindow.ui

DISTFILES += \
    text-speak.png \
    myappico.ico \
    LICENSE \
    myapp.rc

RC_FILE = myapp.rc

HEADERS += \
    mainwindow.h \
    vol/volumemeter.h \
    ../libJMPX/JMPXInterface.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    vol/volumemeter.cpp


