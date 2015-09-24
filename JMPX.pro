# -------------------------------------------------
# Project created by QtCreator 2010-04-13T11:18:57
# -------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JMPX
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    vol/volumemeter.cpp
HEADERS += mainwindow.h \
    vol/volumemeter.h \
    ../libJMPX/JMPXInterface.h
FORMS += mainwindow.ui
OTHER_FILES += 
RESOURCES += icons.qrc
RC_FILE = myapp.rc

DISTFILES += \
    LICENSE \
    myapp.rc
