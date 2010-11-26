include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiinput

QT          += dbus
INCLUDEPATH += ../common/src
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../common/src
CONFIG      += plugin link_pkgconfig
PKGCONFIG   += alsa

FORMS += ../common/src/configuremidiinput.ui \
         ../common/src/configuremidiline.ui

HEADERS += ../common/src/configuremidiinput.h \
           ../common/src/configuremidiline.h \
           ../common/src/midiinputevent.h \
           ../common/src/midiprotocol.h \
           mididevice.h \
           midiinput.h \
           midipoller.h

SOURCES += ../common/src/configuremidiinput.cpp \
           ../common/src/configuremidiline.cpp \
           ../common/src/midiinputevent.cpp \
           ../common/src/midiprotocol.cpp \
           mididevice.cpp \
           midiinput.cpp \
           midipoller.cpp

PRO_FILE = alsa.pro
TRANSLATIONS += MIDI_Input_fi_FI.ts
TRANSLATIONS += MIDI_Input_de_DE.ts
include(../../../i18n.pri)

target.path = $$INSTALLROOT/$$INPUTPLUGINDIR
INSTALLS   += target

