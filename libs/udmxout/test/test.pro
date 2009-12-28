TEMPLATE 	= app
TARGET 		= udmxtest

CONFIG 		+= warn_on
MAKE_CXXFLAGS	+= -Werror

INCLUDEPATH 	+= ../../common
INCLUDEPATH	+= ../unix
DEPENDPATH	+= ../unix

macx:INCLUDEPATH += ../../macx/libusb
macx:LIBS 	+= ../../macx/libusb/libusb.a
macx:LIBS	+= -framework IOKit -framework CoreFoundation
macx:CONFIG	-= app_bundle

HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h

SOURCES += main.cpp \
	   ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp
