TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= llaout

INCLUDEPATH     += . ../../libs/
CONFIG          += plugin link_pkgconfig warn_on release
PKGCONFIG	+= liblla

target.path	= $$OUTPUTPLUGINDIR
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

# Source
HEADERS += configurellaout.h \
	   llaout.h

FORMS += configurellaout.ui

SOURCES += configurellaout.cpp \
	   llaout.cpp
