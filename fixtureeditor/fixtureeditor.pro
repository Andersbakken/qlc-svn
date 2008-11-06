TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc-fixtureeditor

CONFIG          += qt warn_on release
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
unix:LIBS	+= -L../libs/common -lqlccommon
macx:LIBS	+= -L./qlc-fixtureeditor.app/Contents/Frameworks -lqlccommon
win32:release {
	LIBS 	+= -L../libs/common/release/ -lqlccommon
}
win32:debug {
	LIBS 	+= -L../libs/common/debug/ -lqlccommon
}

unix:target.path = /usr/bin
win32:target.path = $$(SystemDrive)/QLC
macx:target.path = /Applications
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

macx {
	system(mkdir -p qlc-fixtureeditor.app/Contents/Frameworks)
	LIBS_PATH = ../libs/common/libqlccommon*.dylib
	system(cp $$LIBS_PATH qlc-fixtureeditor.app/Contents/Frameworks)
	QMAKE_POST_LINK = install_name_tool -change libqlccommon.3.dylib @executable_path/../Frameworks/libqlccommon.3.dylib qlc-fixtureeditor.app/Contents/MacOS/qlc-fixtureeditor
}
