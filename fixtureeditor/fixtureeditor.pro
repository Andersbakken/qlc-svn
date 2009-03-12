include(../variables.pri)

TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor

CONFIG          += qt
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
unix:LIBS	+= -L../libs/common -lqlccommon
win32:LIBS 	+= -L../libs/common/release/ -lqlccommon

# MAC Icon (TODO: Move under ../etc)
macx:ICON	= ../gfx/qlc-fixtureeditor.icns
macx:QMAKE_INFO_PLIST = ./Info.plist

# Bug in QT means it classifies directories as files
macx:QMAKE_DEL_FILE = rm -Rf
macx:QMAKE_CLEAN += qlc-fixtureeditor.app/

# Installation
target.path 	= $$BINDIR
INSTALLS 	+= target

# Sources
RESOURCES += fixtureeditor.qrc
win32:RC_FILE = fixtureeditor.rc

HEADERS += aboutbox.h \
           app.h \
	   capabilitywizard.h \
           editcapability.h \
           editchannel.h \
           editmode.h \
           fixtureeditor.h

FORMS += aboutbox.ui \
	 capabilitywizard.ui \
	 editcapability.ui \
	 editchannel.ui \
	 editmode.ui \
	 fixtureeditor.ui

SOURCES += aboutbox.cpp \
           app.cpp \
	   capabilitywizard.cpp \
           editcapability.cpp \
           editchannel.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp

macx:QMAKE_POST_LINK = ./libupdate.sh
