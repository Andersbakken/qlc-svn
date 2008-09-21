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
win32:HEADERS += usbdmxout-win32.h usbdmx-dynamic.h

SOURCES += configureusbdmxout.cpp
unix:SOURCES += usbdmxout.cpp
win32:SOURCES += usbdmxout-win32.cpp usbdmx-dynamic.cpp
