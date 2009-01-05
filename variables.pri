# Binaries
win32:BINDIR		= $$(SystemDrive)/QLC
unix:BINDIR		= /usr/bin
macx:BINDIR		= /Applications

# Libraries
win32:LIBSDIR		= $$BINDIR
unix:LIBSDIR		= /usr/lib

# Data
win32:DATADIR		= $$(SystemDrive)/QLC
unix:DATADIR		= /usr/share/qlc

# Documentation
win32:DOCSDIR		= $$DATADIR/Documents
unix:DOCSDIR		= $$DATADIR/documents
DEFINES			+= DOCSDIR=\\\"$$DOCSDIR\\\"

# Input devices
win32:INPUTDEVICEDIR	= $$DATADIR/InputDevices
unix:INPUTDEVICEDIR	= $$DATADIR/inputdevices
DEFINES			+= INPUTDEVICEDIR=\\\"$$INPUTDEVICEDIR\\\"

unix:USERINPUTDEVICEDIR	= .qlc/inputdevices
unix:DEFINES		+= USERINPUTDEVICEDIR=\\\"$$USERINPUTDEVICEDIR\\\"

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

