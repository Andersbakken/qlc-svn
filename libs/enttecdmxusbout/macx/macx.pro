include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin

INCLUDEPATH	+= ../../common

CONFIG      += link_pkgconfig
PKGCONFIG   += libftdi

HEADERS     += ../unix/enttecdmxusbout.h \
               ../unix/enttecdmxusbwidget.h \
               ../unix/enttecdmxusbpro.h \
               ../unix/enttecdmxusbopen.h

SOURCES     += ../unix/enttecdmxusbout.cpp \
               ../unix/enttecdmxusbpro.cpp \
               ../unix/enttecdmxusbopen.cpp

# Plugin installation
target.path	 = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

# FTDI library installation
# TODO: Modify to install libftdi.so (from macports) inside the bundle
#QMAKE_POST_LINK += install_name_tool -change /usr/local/lib/libftd2xx.0.1.6.dylib \
#		   @executable_path/../Frameworks/FTDI/libftd2xx.0.1.6.dylib \
#		   libenttecdmxusbout.dylib
#ftdilib.path	= $$INSTALLROOT/$$LIBSDIR/FTDI
#ftdilib.files	= ../ftdi/macx/libftd2xx.0.1.6.dylib
#INSTALLS	+= ftdilib
