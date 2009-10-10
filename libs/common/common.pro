include(../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= qlccommon

CONFIG          += qt static
QT 		+= xml
INCLUDEPATH	+= ../
win32:DEFINES 	+= QLC_EXPORT

# Sources
HEADERS += qlccapability.h \
           qlcchannel.h \
           qlcdocbrowser.h \
           qlcfile.h \
           qlcfixturedef.h \
           qlcfixturedefcache.h \
           qlcfixturemode.h \
	   qlcinputchannel.h \
	   qlcinputprofile.h \
           qlcphysical.h \
           qlctypes.h \
	   qlcwidgetproperties.h

SOURCES += qlccapability.cpp \
           qlcchannel.cpp \
           qlcdocbrowser.cpp \
           qlcfile.cpp \
           qlcfixturedef.cpp \
           qlcfixturedefcache.cpp \
           qlcfixturemode.cpp \
	   qlcinputchannel.cpp \
	   qlcinputprofile.cpp \
           qlcphysical.cpp \
	   qlcwidgetproperties.cpp
