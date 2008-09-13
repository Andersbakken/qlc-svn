TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor
VERSION 	= 3.0

CONFIG          += qt warn_on debug_and_release build_all
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
unix:LIBS	+= -L../libs/common -lqlccommon
win32:release {
LIBS 	+= -L../libs/common/release/ -lqlccommon
}
win32:debug {
LIBS 	+= -L../libs/common/debug/ -lqlccommon
}

unix:target.path = /usr/bin
win32:target.path = C:\QLC
INSTALLS 	+= target

# Sources
RESOURCES += fixtureeditor.qrc
win32:RC_FILE = fixtureeditor.rc

HEADERS += aboutbox.h \
           app.h \
           editcapability.h \
           editchannel.h \
           editmode.h \
           fixtureeditor.h

FORMS += editcapability.ui \
	 editchannel.ui \
	 editmode.ui \
	 fixtureeditor.ui

SOURCES += aboutbox.cpp \
           app.cpp \
           editcapability.cpp \
           editchannel.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp
