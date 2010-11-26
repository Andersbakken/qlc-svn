include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiout

QT          += dbus
INCLUDEPATH += ../common
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../common
CONFIG      += plugin link_pkgconfig
PKGCONFIG   += alsa

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

PRO_FILE = alsa.pro
TRANSLATIONS += MIDI_Output_fi_FI.ts
TRANSLATIONS += MIDI_Output_de_DE.ts
include(../../../i18n.pri)

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
