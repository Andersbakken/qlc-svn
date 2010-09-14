include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcengine

CONFIG 		+= staticlib
CONFIG 		+= qt
macx:CONFIG     -= app_bundle
QT              += core xml
QT		-= gui

INCLUDEPATH	+= ../../plugins/interfaces

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
           collection.h \
           doc.h \
           dummyoutplugin.h \
           efx.h \
           efxfixture.h \
           fixture.h \
           function.h \
           inputmap.h \
           inputpatch.h \
	   intensitygenerator.h \
           mastertimer.h \
           outputmap.h \
           outputpatch.h \
	   palettegenerator.h \
           scene.h

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
           collection.cpp \
           doc.cpp \
           dummyoutplugin.cpp \
           efx.cpp \
           efxfixture.cpp \
           fixture.cpp \
           function.cpp \
           inputmap.cpp \
           inputpatch.cpp \
	   intensitygenerator.cpp \
           mastertimer.cpp \
           outputmap.cpp \
           outputpatch.cpp \
	   palettegenerator.cpp \
           scene.cpp

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
