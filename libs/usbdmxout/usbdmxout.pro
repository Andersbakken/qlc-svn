TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= usbdmxout

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

unix:target.path = /usr/lib/qlc/output
win32:target.path = C:\QLC\Plugins\Output
INSTALLS	+= target

# Sources
FORMS += configureusbdmxout.ui

HEADERS += configureusbdmxout.h
unix:HEADERS += usbdmxout.h
win32:HEADERS += usbdmx-dynamic.h \
		 usbdmxout-win32.h \
		 usbdmxdevice-win32.h

SOURCES += configureusbdmxout.cpp
unix:SOURCES += usbdmxout.cpp
win32:SOURCES += usbdmx-dynamic.cpp \
		 usbdmxout-win32.cpp \
		 usbdmxdevice-win32.cpp
