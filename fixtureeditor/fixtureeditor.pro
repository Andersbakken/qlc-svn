TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor
VERSION 	= 3.0

CONFIG          += qt warn_on release
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
win32:LIBS	+= -L../libs/common/release -lqlccommon
unix:LIBS	+= -L../libs/common -lqlccommon

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
