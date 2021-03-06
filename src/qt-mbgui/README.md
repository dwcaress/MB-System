# Source Directory: src/qt-mbgui/

This directory contains the C++ source files for a prototype library libmbgui that combines a Qt based GUI with modern OpenGL graphics. This library includes Qt/OpenGL classes, GLSL shaders, and QML definitions of the user interface. Eventually, programs built using Qt will replace the X11/Motif based graphical utilities such as mbedit, mbnavedit, mbeditviz, mbgrdviz, mbnavadjust, and mbvelocitytool. As of July 2020, this is just one of several test programs and libraries being used to explore Qt functionality and portability as part of MB-System. None of these are built or installed by default, none will go into general usage, and eventually this directory will disappear. In order to build and install these programs the configure script must be rerun with the --enable-qt argument.

These source files are copyright by David Caress and Dale Chayes and licensed using GPL3 as part of MB-System.
