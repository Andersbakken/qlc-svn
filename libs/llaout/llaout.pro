TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= llaout

INCLUDEPATH     += . ../../libs/
CONFIG          += plugin link_pkgconfig warn_on debug_and_release build_all
PKGCONFIG	+= liblla

unix:target.path	= /usr/lib/qlc/output
win32:target.path	= C:/QLC/Plugins/Output
INSTALLS	+= target

# Source
HEADERS += configurellaout.h \
	   llaout.h

FORMS += configurellaout.ui

SOURCES += configurellaout.cpp \
	   llaout.cpp
