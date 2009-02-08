include(../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= llaout

INCLUDEPATH	+= $$LLA $$LLA/include $$LLA_HTTPD/src/include
LIBS		+= -L $$LLA

INCLUDEPATH     += . ../../libs/
CONFIG          += plugin link_pkgconfig
PKGCONFIG	+= liblla

target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target
LIBS		+= -llla

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

# Source
HEADERS += configurellaout.h \
	   llaout.h

FORMS += configurellaout.ui

SOURCES += configurellaout.cpp \
	   llaout.cpp
