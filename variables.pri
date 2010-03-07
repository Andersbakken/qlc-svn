# Compiler configuration
CONFIG			+= warn_on

# Uncomment these two lines when making a release
# CONFIG			+= release
# CONFIG			-= debug

# Treat all compiler warnings as errors
QMAKE_CXXFLAGS		+= -Werror

# Build universal binaries on Mac OS 10.5 with official Qt 4.5 binary release
macx:CONFIG		+= x86 ppc

# Fix for Mac OS 10.6 with Qt 4.5 (disabled until Qt works fully in Snow Leo)
#macx:CONFIG		-= x86_64
#macx:CONFIG		+= x86

# OLA directories
unix:OLA_GIT		= /usr/src/ola

# Application & version
APPNAME			= Q Light Controller
FXEDNAME		= Fixture Definition Editor
APPVERSION		= 3.0.4

# Install root
win32:INSTALLROOT	= $$(SystemDrive)/QLC
macx:INSTALLROOT	= ~/QLC.app/Contents
unix:!macx:INSTALLROOT += /usr

# Binaries
win32:BINDIR		=
unix:!macx:BINDIR	= bin
macx:BINDIR		= MacOS

# Libraries
win32:LIBSDIR		=
unix:!macx:LIBSDIR	= lib/qlc
macx:LIBSDIR            = Frameworks

# Data
win32:DATADIR		=
unix:!macx:DATADIR	= share/qlc
macx:DATADIR		= Resources

# Documentation
win32:DOCSDIR		= Documents
unix:!macx:DOCSDIR	= $$DATADIR/documents
macx:DOCSDIR		= $$DATADIR/Documents

# Input profiles
win32:INPUTPROFILEDIR	= InputProfiles
unix:!macx:INPUTPROFILEDIR = $$DATADIR/inputprofiles
unix:!macx:USERINPUTPROFILEDIR = .qlc/inputprofiles
macx:INPUTPROFILEDIR	= $$DATADIR/InputProfiles

# Fixtures
win32:FIXTUREDIR	= Fixtures
unix:!macx:FIXTUREDIR	= $$DATADIR/fixtures
unix:!macx:USERFIXTUREDIR = .qlc/fixtures
macx:FIXTUREDIR		= $$DATADIR/Fixtures

# Plugins
win32:PLUGINDIR		= Plugins
unix:!macx:PLUGINDIR	= $$LIBSDIR
macx:PLUGINDIR		= $$DATADIR/Plugins

# Input Plugins
win32:INPUTPLUGINDIR	= $$PLUGINDIR/Input
unix:!macx:INPUTPLUGINDIR = $$PLUGINDIR/input
macx:INPUTPLUGINDIR	= $$PLUGINDIR/Input

# Output Plugins
win32:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output
unix:!macx:OUTPUTPLUGINDIR = $$PLUGINDIR/output
macx:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output
