include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET = olaout

# These don't belong to variables.pri because plugins should not know
# anything about the host app (except what's written in the plugin interface)
# that's loading them, nor vice versa.
#
# OLA Directories
unix:OLA_GIT		= /usr/src/ola
# Should contain google/protobuf/common.h which can be got through
# Macports on Mac
unix:PROTOBUF		= /opt/local/include/

INCLUDEPATH  += . ../../libs $$OLA_GIT $$PROTOBUF
CONFIG  += plugin # link_pkgconfig
#PKGCONFIG += libola

target.path = $$OUTPUTPLUGINDIR
!macx:INSTALLS += target
unix:LIBS += -lola -lolaserver -lolacommon

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

# Forms
FORMS += configureolaout.ui

# Headers
HEADERS += configureolaout.h \
	   olaout.h \
	   olaoutthread.h \
	   qlclogdestination.h

# Source
SOURCES += configureolaout.cpp \
	   olaout.cpp \
	   olaoutthread.cpp \
	   qlclogdestination.cpp

