include (../vars.pro)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= usbdmxout

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

target.path	= $$OUTPUTPLUGINDIR
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

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
