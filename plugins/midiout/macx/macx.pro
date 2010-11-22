include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiout

INCLUDEPATH += ../common
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../common
CONFIG      += plugin
LIBS        += -framework CoreMIDI -framework CoreFoundation

FORMS += ../common/configuremididevice.ui \
         ../common/configuremidiout.ui

HEADERS += ../common/configuremididevice.h \
           ../common/configuremidiout.h \
           mididevice.h \
           midiout.h

SOURCES += ../common/configuremididevice.cpp \
           ../common/configuremidiout.cpp \
           mididevice.cpp \
           midiout.cpp

HEADERS += ../../interfaces/qlcoutplugin.h

PRO_FILE = macx.pro
TRANSLATIONS += MIDI_Output_fi_FI.ts
TRANSLATIONS += MIDI_Output_de_DE.ts
include(../../../i18n.pri)

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
