include(../variables.pri)

TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor

CONFIG          += qt
macx:CONFIG	-= app_bundle
QT 		+= xml

INCLUDEPATH 	+= ../libs/common
DEPENDPATH	+= ../main

unix:LIBS	+= ../libs/common/libqlccommon.a
win32:{
	CONFIG(release, debug|release) LIBS += ../libs/common/release/libqlccommon.a
	CONFIG(debug, debug|release) LIBS += ../libs/common/debug/libqlccommon.a
}

# Installation
target.path 	= $$BINDIR
INSTALLS 	+= target

# Sources
RESOURCES += fixtureeditor.qrc
win32:RC_FILE = fixtureeditor.rc

HEADERS += ../main/aboutbox.h \
           app.h \
	   capabilitywizard.h \
           editcapability.h \
           editchannel.h \
           editmode.h \
           fixtureeditor.h

FORMS += ../main/aboutbox.ui \
	 capabilitywizard.ui \
	 editcapability.ui \
	 editchannel.ui \
	 editmode.ui \
	 fixtureeditor.ui

SOURCES += ../main/aboutbox.cpp \
           app.cpp \
	   capabilitywizard.cpp \
           editcapability.cpp \
           editchannel.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp

