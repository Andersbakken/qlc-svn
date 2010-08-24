# Application & version
APPNAME			= Q Light Controller
FXEDNAME		= Fixture Definition Editor

# Supported image formats for OSX
macx:QTPLUGIN		= qjpeg qgif

# Current application version (not an sh script because of Win-duhs)
APPVERSION		= 3.0.6

# Uncomment these two lines when making a release
CONFIG			+= release
CONFIG			-= debug

# Enable to build universal binaries on Mac OS 10.5
# macx:CONFIG		+= x86 ppc

# Treat all compiler warnings as errors
QMAKE_CXXFLAGS		+= -Werror

# Compiler configuration
CONFIG			+= warn_on

# OLA directories
unix:OLA_GIT		= /usr/src/ola

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
macx:PLUGINDIR		= PlugIns

# Input Plugins
win32:INPUTPLUGINDIR	= $$PLUGINDIR/Input
unix:!macx:INPUTPLUGINDIR = $$PLUGINDIR/input
macx:INPUTPLUGINDIR	= $$PLUGINDIR/Input

# Output Plugins
win32:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output
unix:!macx:OUTPUTPLUGINDIR = $$PLUGINDIR/output
macx:OUTPUTPLUGINDIR	= $$PLUGINDIR/Output

# Translations
win32:TRANSLATIONDIR	=
unix:!macx:TRANSLATIONDIR = $$DATADIR/translations
macx:TRANSLATIONDIR	= $$DATADIR/Translations
