TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= usbdmxout

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

unix:target.path = /usr/lib/qlc/output
win32:target.path = C:/QLC/Plugins/Output
INSTALLS	+= target

# Forms
FORMS += configureusbdmxout.ui

# Headers
unix:HEADERS += configureusbdmxout.h \
                usbdmxout-unix.h \
                usbdmxdevice-unix.h

win32:HEADERS += configureusbdmxout.h \
                 usbdmx-dynamic.h \
		 usbdmxout-win32.h \
		 usbdmxdevice-win32.h

# Sources
unix:SOURCES += configureusbdmxout.cpp \
                usbdmxout-unix.cpp \
                usbdmxdevice-unix.cpp

win32:SOURCES += configureusbdmxout.cpp \
                 usbdmx-dynamic.cpp \
		 usbdmxout-win32.cpp \
		 usbdmxdevice-win32.cpp
