# Compiler configuration
CONFIG			+= warn_on release
CONFIG			-= debug

# Fix for Mac OS 10.6 with Qt 4.5
macx:CONFIG		-= x86_64
macx:CONFIG		+= x86

# OLA directories
unix:OLA_GIT		= /usr/src/ola

# Treat all compiler warnings as errors
# QMAKE_CXXFLAGS		+= -Werror

# Binaries
win32:BINDIR		= $$(SystemDrive)/QLC
unix:!macx:BINDIR	= /usr/bin
macx:BINDIR		= ~/qlc.app/Contents/MacOS

# Libraries
win32:LIBSDIR		= $$BINDIR
unix:!macx:LIBSDIR	= /usr/lib
macx:LIBSDIR            = ~/qlc.app/Contents/Frameworks/

# Data
win32:DATADIR		= $$(SystemDrive)/QLC
unix:!macx:DATADIR	= /usr/share/qlc
macx:DATADIR		= ~/qlc.app/Contents/Resources/

# Documentation
win32:DOCSDIR		= $$DATADIR/Documents
win32:DEFINES		+= DOCSDIR=\\\"Documents\\\"

unix:!macx:DOCSDIR	= $$DATADIR/documents
unix:!macx:DEFINES	+= DOCSDIR=\\\"$$DOCSDIR\\\"

macx:DOCSDIR		= $$DATADIR/Documents/
macx:DEFINES		+= DOCSDIR=\\\"../Resources/Documents\\\"

# Input profiles
win32:INPUTPROFILEDIR	= $$DATADIR/InputProfiles
win32:DEFINES		+= INPUTPROFILEDIR=\\\"InputProfiles\\\"

unix:!macx:INPUTPROFILEDIR	= $$DATADIR/inputprofiles
unix:!macx:DEFINES		+= INPUTPROFILEDIR=\\\"$$INPUTPROFILEDIR\\\"
unix:!macx:USERINPUTPROFILEDIR	= .qlc/inputprofiles
unix:!macx:DEFINES		+= USERINPUTPROFILEDIR=\\\"$$USERINPUTPROFILEDIR\\\"

macx:INPUTPROFILEDIR	= $$DATADIR/InputProfiles
macx:DEFINES		+= INPUTPROFILEDIR=\\\"../Resources/InputProfiles\\\"

# Fixtures
win32:FIXTUREDIR	= $$DATADIR/Fixtures
win32:DEFINES		+= FIXTUREDIR=\\\"Fixtures\\\"

unix:!macx:FIXTUREDIR		= $$DATADIR/fixtures
unix:!macx:DEFINES		+= FIXTUREDIR=\\\"$$FIXTUREDIR\\\"
unix:!macx:USERFIXTUREDIR	= .qlc/fixtures
unix:!macx:DEFINES		+= USERFIXTUREDIR=\\\"$$USERFIXTUREDIR\\\"

macx:FIXTUREDIR		= $$DATADIR/Fixtures
macx:DEFINES		+= FIXTUREDIR=\\\"../Resources/Fixtures\\\"

# Plugins
win32:PLUGINDIR		= $$LIBSDIR/Plugins
unix:!macx:PLUGINDIR	= $$LIBSDIR/qlc
macx:PLUGINDIR		= $$DATADIR/../Plugins

# Input Plugins
win32:INPUTPLUGINDIR	= $$PLUGINDIR/Input
win32:DEFINES		+= INPUTPLUGINDIR=\\\"Plugins/Input\\\"

unix:!macx:INPUTPLUGINDIR = $$PLUGINDIR/input
unix:!macx:DEFINES	+= INPUTPLUGINDIR=\\\"$$INPUTPLUGINDIR\\\"

macx:INPUTPLUGINDIR	= $$PLUGINDIR/Input
macx:DEFINES		+= INPUTPLUGINDIR=\\\"../Plugins/Input\\\"

# Output Plugins
win32:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output
win32:DEFINES		+= OUTPUTPLUGINDIR=\\\"Plugins/Output\\\"

unix:!macx:OUTPUTPLUGINDIR = $$PLUGINDIR/output
unix:!macx:DEFINES	+= OUTPUTPLUGINDIR=\\\"$$OUTPUTPLUGINDIR\\\"

macx:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output
macx:DEFINES		+= OUTPUTPLUGINDIR=\\\"../Plugins/Output\\\"
