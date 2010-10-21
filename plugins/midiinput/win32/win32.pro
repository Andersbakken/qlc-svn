include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiinput

CONFIG      += plugin
INCLUDEPATH += ../common
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../common
LIBS        += -lwinmm

###############################################################################
# Installation
###############################################################################

target.path = $$INSTALLROOT/$$INPUTPLUGINDIR
INSTALLS   += target

###############################################################################
# Sources
###############################################################################

FORMS += ../common/configuremidiinput.ui \
         ../common/configuremidiline.ui

HEADERS += ../common/configuremidiinput.h \
           ../common/configuremidiline.h \
           ../common/midiinputevent.h \
           ../common/midiprotocol.h \
           mididevice.h \
           midiinput.h

SOURCES += ../common/configuremidiinput.cpp \
           ../common/configuremidiline.cpp \
           ../common/midiinputevent.cpp \
           ../common/midiprotocol.cpp \
           mididevice.cpp \
           midiinput.cpp

HEADERS += ../../interfaces/qlcinplugin.h
