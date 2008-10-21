TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= serialdmx

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

unix:target.path = /usr/lib/qlc/output
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

# Forms
FORMS += configureserialdmx.ui

# Headers
unix:HEADERS += configureserialdmx.h \
                serialdmx.h \
		serialdmxdevice.h \
		ftdidevice.h

# Sources
unix:SOURCES += configureserialdmx.cpp \
                serialdmx.cpp \
		serialdmxdevice.cpp \
		ftdidevice.c

macx:QMAKE_POST_LINK = cp libftd2xx.0.1.4.dylib ../../main/qlc.app/Contents/Plugins/output/