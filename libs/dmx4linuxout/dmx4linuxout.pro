TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= dmx4linuxout

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on debug_and_release build_all

unix:target.path	= /usr/lib/qlc/output
win32:target.path	= C:/QLC/Plugins/Output
INSTALLS	+= target

# Sources
HEADERS += configuredmx4linuxout.h \
	   dmx4linuxout.h

FORMS += configuredmx4linuxout.ui

SOURCES += configuredmx4linuxout.cpp \
	   dmx4linuxout.cpp
