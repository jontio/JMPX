TEMPLATE = subdirs

CONFIG += ordered

include(common.pri)

print_user_info_msgs {
debug {
    message("debug is on: edit common.pri to change it")
}
release {
    message("release is on: edit common.pri to change it")
}
message("You will need Qt5 and libopus to compile JMPX.")
message("try \"sudo apt-get install libopus-dev\" and \"sudo apt-get install qt5-default\" on linux; that may work")
message("on linux select your sound systems in libJMPX/libJMPX.pro and install the required sound system libries")
message("on linux selecting only __UNIX_JACK__ is fine and you don't need the others")
message("type \"make\" to build JMPX")
message("after that is done type \"make install\" to install JMPX")
message("or type \"cd build\" then \"./JMPX\" to run JMPX without installing")
}

SUBDIRS = libJMPX \
          JMPX

JMPX.depends = libJMPX
