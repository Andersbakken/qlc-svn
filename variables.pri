# Binaries
win32:BINDIR		= $$(SystemDrive)/QLC
unix:BINDIR		= /usr/bin
macx:BINDIR		= /Applications

# Libraries
win32:LIBSDIR		= $$QLCDIR
unix:LIBSDIR		= /usr/lib

# Data
win32:DATADIR		= $$(SystemDrive)/QLC
unix:DATADIR		= /usr/share/qlc

# Input Templates
win32:INPUTTEMPLATEDIR	= $$DATADIR/InputTemplates
unix:INPUTTEMPLATEDIR	= $$DATADIR/inputtemplates
DEFINES			+= INPUTTEMPLATEDIR=\\\"$$INPUTTEMPLATEDIR\\\"

unix:USERINPUTTEMPLATEDIR	= .qlc/inputtemplates
unix:DEFINES		+= USERINPUTTEMPLATEDIR=\\\"$$USERINPUTTEMPLATEDIR\\\"

# Fixtures
win32:FIXTUREDIR	= $$DATADIR/Fixtures
unix:FIXTUREDIR		= $$DATADIR/fixtures
DEFINES			+= FIXTUREDIR=\\\"$$FIXTUREDIR\\\"

unix:USERFIXTUREDIR	= .qlc/fixtures
unix:DEFINES		+= USERFIXTUREDIR=\\\"$$USERFIXTUREDIR\\\"

# Plugins
win32:PLUGINDIR		= $$LIBSDIR/Plugins
unix:PLUGINDIR		= $$LIBSDIR/qlc

#Input Plugins
unix:INPUTPLUGINDIR	= $$PLUGINDIR/input
win32:INPUTPLUGINDIR	= $$PLUGINDIR/Input
macx:INPUTPLUGINDIR	= ../Plugins/Input
DEFINES			+= INPUTPLUGINDIR=\\\"$$INPUTPLUGINDIR\\\"

# Output Plugins
unix:OUTPUTPLUGINDIR	= $$PLUGINDIR/output
win32:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output
macx:OUTPUTPLUGINDIR	= ../Plugins/Output
DEFINES			+= OUTPUTPLUGINDIR=\\\"$$OUTPUTPLUGINDIR\\\"

