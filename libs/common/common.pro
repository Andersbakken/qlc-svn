include(../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= qlccommon
VERSION 	= 3.0

CONFIG          += qt warn_on release
QT 		+= xml
INCLUDEPATH	+= ../
win32:DEFINES 	+= QLC_EXPORT

# Sources
HEADERS += qlccapability.h \
           qlcchannel.h \
           qlcdocbrowser.h \
           qlcfile.h \
           qlcfixturedef.h \
           qlcfixturemode.h \
	   qlcinputchannel.h \
	   qlcinputdevice.h \
           qlcphysical.h \
           qlctypes.h
	   
SOURCES += qlccapability.cpp \
           qlcchannel.cpp \
           qlcdocbrowser.cpp \
           qlcfile.cpp \
           qlcfixturedef.cpp \
           qlcfixturemode.cpp \
	   qlcinputchannel.cpp \
	   qlcinputdevice.cpp \
           qlcphysical.cpp

# Installation
target.path	= $$LIBSDIR

unix:headers.path = /usr/include/qlc
win32:headers.path = $$LIBSDIR/include
headers.files = qlccapability.h \
                qlcchannel.h \
                qlcdocbrowser.h \
                qlcfile.h \
                qlcfixturedef.h \
                qlcfixturemode.h \
		qlcinputchannel.h \
		qlcinputdevice.h \
                qlcphysical.h \
                qlctypes.h

INSTALLS 	+= target headers
