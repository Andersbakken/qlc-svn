include(../../variables.pri)
include(../../coverage.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcengine

CONFIG  += qt
QT      += core xml
QT      -= gui
QTPLUGIN =

INCLUDEPATH += ../../plugins/interfaces

#############################################################################
# Installation
#############################################################################

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../macx/nametool.pri)
}

target.path = $$INSTALLROOT/$$LIBSDIR
INSTALLS   += target

#############################################################################
# Sources
#############################################################################

# Fixture metadata
HEADERS += qlccapability.h \
           qlcchannel.h \
           qlcfile.h \
           qlcfixturedef.h \
           qlcfixturedefcache.h \
           qlcfixturemode.h \
           qlci18n.h \
           qlcinputchannel.h \
           qlcinputprofile.h \
           qlcphysical.h

# Engine
HEADERS += bus.h \
           chaser.h \
           chaserrunner.h \
           collection.h \
           doc.h \
           efx.h \
           efxfixture.h \
           fadechannel.h \
           fixture.h \
           function.h \
           inputmap.h \
           inputpatch.h \
           intensitygenerator.h \
           mastertimer.h \
           universearray.h \
           outputmap.h \
           outputpatch.h \
           palettegenerator.h \
           scene.h \
           scenevalue.h

# Fixture metadata
SOURCES += qlccapability.cpp \
           qlcchannel.cpp \
           qlcfile.cpp \
           qlcfixturedef.cpp \
           qlcfixturedefcache.cpp \
           qlcfixturemode.cpp \
           qlci18n.cpp \
           qlcinputchannel.cpp \
           qlcinputprofile.cpp \
           qlcphysical.cpp

# Engine
SOURCES += bus.cpp \
           chaser.cpp \
           chaserrunner.cpp \
           collection.cpp \
           doc.cpp \
           efx.cpp \
           efxfixture.cpp \
           fadechannel.cpp \
           fixture.cpp \
           function.cpp \
           inputmap.cpp \
           inputpatch.cpp \
           intensitygenerator.cpp \
           mastertimer.cpp \
           universearray.cpp \
           outputmap.cpp \
           outputpatch.cpp \
           palettegenerator.cpp \
           scene.cpp \
           scenevalue.cpp

# Interfaces
HEADERS += ../../plugins/interfaces/qlcinplugin.h \
           ../../plugins/interfaces/qlcoutplugin.h

#############################################################################
# qlcconfig.h generation
#############################################################################

CONFIGFILE = qlcconfig.h
conf.target = $$CONFIGFILE
QMAKE_EXTRA_TARGETS += conf
PRE_TARGETDEPS += $$CONFIGFILE
QMAKE_CLEAN += $$CONFIGFILE
QMAKE_DISTCLEAN += $$CONFIGFILE

macx {
    conf.commands += echo \"$$LITERAL_HASH ifndef CONFIG_H\" > $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define CONFIG_H\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPNAME \\\"$$APPNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FXEDNAME \\\"$$FXEDNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPVERSION \\\"$$APPVERSION\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define DOCSDIR \\\"$$DOCSDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define INPUTPROFILEDIR \\\"$$INPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERINPUTPROFILEDIR \\\"$$USERINPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FIXTUREDIR \\\"$$FIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERFIXTUREDIR \\\"$$USERFIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define INPUTPLUGINDIR \\\"$$INPUTPLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define OUTPUTPLUGINDIR \\\"$$OUTPUTPLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH endif\" >> $$CONFIGFILE
}
unix:!macx {
    conf.commands += echo \"$$LITERAL_HASH ifndef CONFIG_H\" > $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define CONFIG_H\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPNAME \\\"$$APPNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FXEDNAME \\\"$$FXEDNAME\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define APPVERSION \\\"$$APPVERSION\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define DOCSDIR \\\"$$INSTALLROOT/$$DOCSDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define INPUTPROFILEDIR \\\"$$INSTALLROOT/$$INPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERINPUTPROFILEDIR \\\"$$USERINPUTPROFILEDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define FIXTUREDIR \\\"$$INSTALLROOT/$$FIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define USERFIXTUREDIR \\\"$$USERFIXTUREDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define INPUTPLUGINDIR \\\"$$INSTALLROOT/$$INPUTPLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define OUTPUTPLUGINDIR \\\"$$INSTALLROOT/$$OUTPUTPLUGINDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH define TRANSLATIONDIR \\\"$$INSTALLROOT/$$TRANSLATIONDIR\\\"\" >> $$CONFIGFILE &&
    conf.commands += echo \"$$LITERAL_HASH endif\" >> $$CONFIGFILE
}
win32 {
    conf.commands += @echo $$LITERAL_HASH ifndef CONFIG_H > $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define CONFIG_H >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define APPNAME \"$$APPNAME\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define FXEDNAME \"$$FXEDNAME\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define APPVERSION \"$$APPVERSION\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define DOCSDIR \"$$DOCSDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define INPUTPROFILEDIR \"$$INPUTPROFILEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERINPUTPROFILEDIR \"$$USERINPUTPROFILEDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define FIXTUREDIR \"$$FIXTUREDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define USERFIXTUREDIR \"$$USERFIXTUREDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define INPUTPLUGINDIR \"$$INPUTPLUGINDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define OUTPUTPLUGINDIR \"$$OUTPUTPLUGINDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH define TRANSLATIONDIR \"$$TRANSLATIONDIR\" >> $$CONFIGFILE &&
    conf.commands += @echo $$LITERAL_HASH endif >> $$CONFIGFILE
}
