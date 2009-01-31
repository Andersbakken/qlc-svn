# Compiler configuration
CONFIG			+= warn_on release

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
win32:DEFINES		+= DOCSDIR=\\\"Documents\\\"

unix:DOCSDIR		= $$DATADIR/documents
unix:DEFINES		+= DOCSDIR=\\\"$$DOCSDIR\\\"

# Input devices
win32:INPUTDEVICEDIR	= $$DATADIR/InputDevices
win32:DEFINES		+= INPUTDEVICEDIR=\\\"InputDevices\\\"

unix:INPUTDEVICEDIR	= $$DATADIR/inputdevices
unix:DEFINES		+= INPUTDEVICEDIR=\\\"$$INPUTDEVICEDIR\\\"
unix:USERINPUTDEVICEDIR	= .qlc/inputdevices
unix:DEFINES		+= USERINPUTDEVICEDIR=\\\"$$USERINPUTDEVICEDIR\\\"

# Fixtures
win32:FIXTUREDIR	= $$DATADIR/Fixtures
win32:DEFINES		+= FIXTUREDIR=\\\"Fixtures\\\"

unix:FIXTUREDIR		= $$DATADIR/fixtures
unix:DEFINES		+= FIXTUREDIR=\\\"$$FIXTUREDIR\\\"
unix:USERFIXTUREDIR	= .qlc/fixtures
unix:DEFINES		+= USERFIXTUREDIR=\\\"$$USERFIXTUREDIR\\\"

# Plugins
win32:PLUGINDIR		= $$LIBSDIR/Plugins
unix:PLUGINDIR		= $$LIBSDIR/qlc

# Input Plugins
win32:INPUTPLUGINDIR	= $$PLUGINDIR/Input
win32:DEFINES		+= INPUTPLUGINDIR=\\\"Plugins/Input\\\"

unix:INPUTPLUGINDIR	= $$PLUGINDIR/input
macx:INPUTPLUGINDIR	= ../Plugins/Input
unix:macx:DEFINES	+= INPUTPLUGINDIR=\\\"$$INPUTPLUGINDIR\\\"

# Output Plugins
win32:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output
win32:DEFINES		+= OUTPUTPLUGINDIR=\\\"Plugins/Output\\\"

unix:OUTPUTPLUGINDIR	= $$PLUGINDIR/output
macx:OUTPUTPLUGINDIR	= ../Plugins/Output
!win32:DEFINES		+= OUTPUTPLUGINDIR=\\\"$$OUTPUTPLUGINDIR\\\"
