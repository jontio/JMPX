#JMPX
A Stereo encoder for FM transmitters.

![Screenshot of JMPX](http://jontio.zapto.org/hda1/paradise/jmpxencoder/jmpxencoder.jpeg)

This program can be used in conjunction with any FM transmitter to produce stereo transmissions. This allows you to transmit your very own FM stereo signal which can be received with any standard FM stereo radio. The FM transmitter gets connected to the computer soundcard and JMPX takes care of the rest.

This program consists of a dynamically liked library and a GUI. The library performs the main work of obtaining audio and producing audio, and the GUI is just for control and looks. A soundcard that supports at least 96000bps is needed.

##Compiling

Qt and Qt Creator are recommended.

* Open [libJMPX/libJMPX.pro](libJMPX/libJMPX.pro) with Qt Creator and build
* Open [JMPX/JMPX.pro](JMPX/JMPX.pro) with Qt Creator and build
* Copy libJMPX.dll (or something like liblibJMPX.so.0.1 on Linux) from the build directory when building libJMPX to the build directory when building JMPX
* Run JMPX.exe (or JMPX on Linux)


Jonti 2015
http://jontio.zapto.org
