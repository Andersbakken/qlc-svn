include(../../../variables.pri)
include(../../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = ewinginput

INCLUDEPATH   += ../../interfaces
CONFIG        += plugin
QT            += network
win32:DEFINES += QLC_EXPORT
QTPLUGIN       =

win32 {
    # Qt Libraries
    qtnetwork.path = $$INSTALLROOT/$$LIBSDIR
    CONFIG(release, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetwork4.dll
    CONFIG(debug, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetworkd4.dll
    INSTALLS    += qtnetwork
}

# Input
HEADERS += ewinginput.h \
           eplaybackwing.h \
           eshortcutwing.h \
           eprogramwing.h \
           ewing.h

SOURCES += ewinginput.cpp \
           eplaybackwing.cpp \
           eshortcutwing.cpp \
           eprogramwing.cpp \
           ewing.cpp

HEADERS += ../../interfaces/qlcinplugin.h

PRO_FILE = src.pro
TRANSLATIONS += ENTTEC_Wing_Input_fi_FI.ts
TRANSLATIONS += ENTTEC_Wing_Input_de_DE.ts
include(../../../i18n.pri)

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../../macx/nametool.pri)
}

target.path = $$INSTALLROOT/$$INPUTPLUGINDIR
INSTALLS   += target

