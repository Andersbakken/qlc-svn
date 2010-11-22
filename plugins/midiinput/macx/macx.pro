include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiinput

CONFIG      += plugin
INCLUDEPATH += ../common/src
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../common/src
LIBS        += -framework CoreMIDI -framework CoreFoundation

FORMS += ../common/src/configuremidiinput.ui \
         ../common/src/configuremidiline.ui

HEADERS += ../common/src/midiinputevent.h \
           ../common/src/configuremidiinput.h \
           ../common/src/configuremidiline.h \
           ../common/src/midiprotocol.h \
           mididevice.h \
           midiinput.h

SOURCES += ../common/src/midiinputevent.cpp \
           ../common/src/configuremidiinput.cpp \
           ../common/src/configuremidiline.cpp \
           ../common/src/midiprotocol.cpp \
           mididevice.cpp \
           midiinput.cpp

HEADERS += ../../interfaces/qlcinplugin.h

PRO_FILE = macx.pro
TRANSLATIONS += MIDI_Input_fi_FI.ts
TRANSLATIONS += MIDI_Input_de_DE.ts
include(../../../i18n.pri)

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$INPUTPLUGINDIR
INSTALLS   += target
