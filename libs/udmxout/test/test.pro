TEMPLATE 	= app
TARGET 		= udmxtest

CONFIG 		+= warn_on
MAKE_CXXFLAGS	+= -Werror

macx:CONFIG	-= app_bundle

INCLUDEPATH 	+= ../../ ../unix
macx:INCLUDEPATH += ../../macx/libusb
macx:LIBS 	+= ../../macx/libusb/libusb.a -lIOKit

HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h

SOURCES += main.cpp \
	   ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp
