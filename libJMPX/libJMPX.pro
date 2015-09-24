#-------------------------------------------------
#
# Project created by QtCreator 2015-09-22T10:02:32
#
#-------------------------------------------------

QT       -= gui

TARGET = libJMPX
TEMPLATE = lib

INCLUDEPATH += ../rtaudio-4.1.1/include \
    ../kiss_fft130

DEFINES += JMPX_LIBRARY \
        REAL_FASTFIR \
        "kiss_fft_scalar=double"

win32 {
    DEFINES += __WINDOWS_DS__
    #  not sure what the advantages for WASAPI is over DS DEFINES += __WINDOWS_WASAPI__
}
unix {
    DEFINES += __UNIX_JACK__ \
            __LINUX_PULSE__ \
            __LINUX_ALSA__
}

SOURCES += libJMPX.cpp \
    JSound.cpp \
    JDSP.cpp \
    ../rtaudio-4.1.1/RtAudio.cpp \
    ../kiss_fft130/tools/kiss_fastfir.c \
    ../kiss_fft130/kiss_fft.c \
    ../kiss_fft130/tools/kiss_fftr.c

HEADERS += libJMPX.h \
    Definitions.h \
    JSound.h \
    JDSP.h \
    JMPXInterface.h \
    ../rtaudio-4.1.1/RtAudio.h \
    ../kiss_fft130/tools/kiss_fftr.h \
    ../kiss_fft130/tools/kiss_fastfir.h \
    ../kiss_fft130/_kiss_fft_guts.h \
    ../kiss_fft130/kiss_fft.h


contains(DEFINES, __WINDOWS_DS__) {
    LIBS += libdsound libole32 libwinmm
}

contains(DEFINES, __WINDOWS_WASAPI__) {
    LIBS += libole32 libwinmm libksuser libuuid
}

contains(DEFINES, __UNIX_JACK__) {
    LIBS += libjack libpthread
}

contains(DEFINES, __LINUX_PULSE__) {
    LIBS += libpthread libpulse-simple libpulse
}

contains(DEFINES, __LINUX_ALSA__ ) {
    LIBS += libasound libpthread
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    LICENSE \
    ../kiss_fft130/README \
    ../kiss_fft130/TIPS \
    ../kiss_fft130/CHANGELOG \
    ../kiss_fft130/COPYING \
    ../rtaudio-4.1.1/readme
