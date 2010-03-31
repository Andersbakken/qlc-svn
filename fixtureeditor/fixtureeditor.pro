include(../variables.pri)

TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor

CONFIG          += qt
macx:CONFIG	-= app_bundle
QT 		+= xml

INCLUDEPATH 	+= ../libs/common
DEPENDPATH	+= ../libs/common

INCLUDEPATH	+= ../engine/src
DEPENDPATH	+= ../engine/src

INCLUDEPATH	+= ../ui/src
DEPENDPATH	+= ../ui/src

unix:LIBS	+= ../libs/common/libqlccommon.a
win32:{
	CONFIG(release, debug|release) LIBS += ../libs/common/release/libqlccommon.a
	CONFIG(debug, debug|release) LIBS += ../libs/common/debug/libqlccommon.a
}

# Installation
target.path 	= $$INSTALLROOT/$$BINDIR
INSTALLS 	+= target

# Sources
RESOURCES += fixtureeditor.qrc
win32:RC_FILE = fixtureeditor.rc

HEADERS += ../ui/src/aboutbox.h \
	   ../ui/src/docbrowser.h \
           app.h \
	   capabilitywizard.h \
           editcapability.h \
           editchannel.h \
           editmode.h \
           fixtureeditor.h

FORMS += ../ui/src/aboutbox.ui \
	 capabilitywizard.ui \
	 editcapability.ui \
	 editchannel.ui \
	 editmode.ui \
	 fixtureeditor.ui

SOURCES += ../ui/src/aboutbox.cpp \
	   ../ui/src/docbrowser.cpp \
           app.cpp \
	   capabilitywizard.cpp \
           editcapability.cpp \
           editchannel.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp

