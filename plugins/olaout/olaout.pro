include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = olaout

# These don't belong to variables.pri because plugins should not know
# anything about the host app (except what's written in the plugin interface)
# that's loading them, nor vice versa.
#
# OLA Directories
unix:OLA_GIT = /usr/src/ola

# Should contain google/protobuf/common.h which can be got through
# Macports on Mac
unix:PROTOBUF = /opt/local/include/

INCLUDEPATH += ../interfaces
INCLUDEPATH += $$OLA_GIT $$PROTOBUF
unix:LIBS   += -lola -lolaserver -lolacommon

CONFIG += plugin # link_pkgconfig
CONFIG -= ppc # LibOLA is not a universal binary 
#PKGCONFIG += libola

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

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
