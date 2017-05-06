#-------------------------------------------------
#
# Project created by QtCreator 2015-09-22T10:02:32
#
#-------------------------------------------------

#this file always seems to need a few tweaks to get it working right on all platforms. you might have to do a little.

include(../common.pri)

QT       -= gui

TARGET = libJMPX
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11 -ffast-math
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_CFLAGS += -ffast-math
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3

INCLUDEPATH += ../rtaudio-4.1.2/include \
    ../kiss_fft130

DEFINES += JMPX_LIBRARY \
        REAL_FASTFIR \
        "kiss_fft_scalar=double"

#remove warnings about std::auto_ptr being deprescated. what should we replace them with. TODO
QMAKE_CXXFLAGS += -Wno-deprecated


#libopus is here.
INCLUDEPATH +=../libopus-1.2-alpha/include
win32 {
contains(QT_ARCH, i386) {
    #message("32-bit")
    LIBS += -L$$PWD/../libopus-1.2-alpha/lib/32
} else {
    #message("64-bit")
    LIBS += -L$$PWD/../libopus-1.2-alpha/lib/64
}
LIBS += -llibopus
}
unix {
#libopus library
LIBS += -lopus
}

#here we have rtaudio
#You dont need all these audio APIs.
#Say if you want only jack then remove __LINUX_PULSE__ and __LINUX_ALSA__ leaving just DEFINES += __UNIX_JACK__
#I have never compiled on mac so if you have let me know if anything should be changed
win32 {
    DEFINES += __WINDOWS_DS__
    #  not sure what the advantages for WASAPI is over DS DEFINES += __WINDOWS_WASAPI__
}
unix {
#select your wanted sound systems
    DEFINES += __UNIX_JACK__
#    DEFINES += __LINUX_PULSE__
#    DEFINES += __LINUX_ALSA__
}
macx {
    DEFINES += __MACOSX_CORE__ \
            __UNIX_JACK__
}

SOURCES += libJMPX.cpp \
    JSound.cpp \
    JDSP.cpp \
    ../rtaudio-4.1.2/RtAudio.cpp \
    ../kiss_fft130/tools/kiss_fastfir.c \
    ../kiss_fft130/kiss_fft.c \
    ../kiss_fft130/tools/kiss_fftr.c \
    rds.cpp \
    oqpskmodulator.cpp \
    dataformatter.cpp \
    ../viterbi-xukmin/viterbi.cpp

HEADERS += libJMPX.h \
    Definitions.h \
    JSound.h \
    JDSP.h \
    JMPXInterface.h \
    ../rtaudio-4.1.2/RtAudio.h \
    ../kiss_fft130/tools/kiss_fftr.h \
    ../kiss_fft130/tools/kiss_fastfir.h \
    ../kiss_fft130/_kiss_fft_guts.h \
    ../kiss_fft130/kiss_fft.h \
    rds.h \
    oqpskmodulator.h \
    dataformatter.h \
    ../viterbi-xukmin/viterbi.h \
    ../libopus-1.2-alpha/include/opus/opus.h


contains(DEFINES, __WINDOWS_DS__) {
    LIBS += libdsound libole32 libwinmm
}

contains(DEFINES, __WINDOWS_WASAPI__) {
    LIBS += libole32 libwinmm libksuser libuuid
}

contains(DEFINES, __UNIX_JACK__) {
    LIBS += -ljack -lpthread
}

contains(DEFINES, __LINUX_PULSE__) {
    LIBS += -lpthread -lpulse-simple -lpulse
}

contains(DEFINES, __LINUX_ALSA__ ) {
    LIBS += -lasound -lpthread
}

#are these the right things to link with for coreaudio?
contains(DEFINES, __MACOSX_CORE__ ) {
    LIBS += -lCoreAudio -lpthread
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}

use_build_dir {
debug {
    DESTDIR = $$PWD/../build_debug
print_user_info_msgs {
    message("using debug build dir")
}
} else {
    DESTDIR = $$PWD/../build
print_user_info_msgs {
    message("using release build dir")
}
}
}


DISTFILES += \
    LICENSE \
    ../kiss_fft130/README \
    ../kiss_fft130/TIPS \
    ../kiss_fft130/CHANGELOG \
    ../kiss_fft130/COPYING \
    ../rtaudio-4.1.2/readme \
    ../viterbi-xukmin/LICENSE.md \
    ../viterbi-xukmin/README.md
