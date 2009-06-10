include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET = llaout

# These don't belong to variables.pri because plugins should not know
# anything about the host app (except what's written in the plugin interface)
# that's loading them, nor vice versa.
#
# LLA Directories
unix:LLA_GIT		= /usr/src/lla
# Should contain google/protobuf/common.h which can be got through
# Macports on Mac
unix:PROTOBUF		= /opt/local/include/

INCLUDEPATH  += . ../../libs $$LLA_GIT $$PROTOBUF
CONFIG  += plugin # link_pkgconfig
#PKGCONFIG += liblla

target.path = $$OUTPUTPLUGINDIR
!macx:INSTALLS += target
unix:LIBS += -llla -lllaserver -lllacommon

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

# Forms
FORMS += configurellaout.ui

# Headers
HEADERS += configurellaout.h \
	   llaout.h \
	   llaoutthread.h \
	   qlclogdestination.h

# Source
SOURCES += configurellaout.cpp \
	   llaout.cpp \
	   llaoutthread.cpp \
	   qlclogdestination.cpp

