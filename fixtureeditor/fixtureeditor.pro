include(../variables.pri)

TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor

CONFIG          += qt
macx:CONFIG	-= app_bundle
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
unix:LIBS	+= ../libs/common/libqlccommon.a
win32:{
	# Windows is so fucking retarded that you can't link these statically
	release:LIBS 	+= -L../libs/common/release -lqlccommon
	debug:LIBS 	+= -L../libs/common/debug -lqlccommon
}

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

