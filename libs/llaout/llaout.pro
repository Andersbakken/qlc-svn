include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET = llaout

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

