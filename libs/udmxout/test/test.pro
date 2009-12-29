include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET 	 = udmxtest

CONFIG -= app_bundle

INCLUDEPATH += ../../common
INCLUDEPATH += ../unix
DEPENDPATH  += ../unix

macx:INCLUDEPATH += ../../macx/libusb
macx:LIBS 	 += ../../macx/libusb/libusb.a
macx:LIBS	 += -framework IOKit -framework CoreFoundation

HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h

SOURCES += main.cpp \
	   ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp
