TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= llaout

INCLUDEPATH     += . ../../libs/
CONFIG          += plugin link_pkgconfig warn_on debug_and_release build_all
PKGCONFIG	+= liblla

target.path	= /usr/lib/qlc/output
INSTALLS	+= target

# Source
HEADERS += configurellaout.h \
	   llaout.h

FORMS += configurellaout.ui

SOURCES += configurellaout.cpp \
	   llaout.cpp
