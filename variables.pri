#############################################################################
# Application name & version
#############################################################################

APPNAME    = Q Light Controller
FXEDNAME   = Fixture Definition Editor
APPVERSION = 3.1.0

#############################################################################
# Compiler & linker configuration
#############################################################################

QMAKE_CXXFLAGS += -Werror
CONFIG         += warn_on
macx:CONFIG    -= app_bundle # Let QLC construct the .app bundle

#CONFIG        += release # Enable this when making a release
#CONFIG        -= debug   # Disable this when making a release

unix:OLA_GIT    = /usr/src/ola    # OLA directories

#macx:CONFIG   += x86 ppc  # Build universal binaries (Leopard only)
#CONFIG        += coverage # Unit test coverage measurement

# Produce build targets to the source directory
win32:DESTDIR  = ./

#############################################################################
# Installation paths
#############################################################################

# Install root
win32:INSTALLROOT       = $$(SystemDrive)/QLC
macx:INSTALLROOT        = ~/QLC.app/Contents
unix:!macx:INSTALLROOT += /usr

# Binaries
win32:BINDIR      =
unix:!macx:BINDIR = bin
macx:BINDIR       = MacOS

# Libraries
win32:LIBSDIR      =
unix:!macx:LIBSDIR = lib
macx:LIBSDIR       = Frameworks

# Data
win32:DATADIR      =
unix:!macx:DATADIR = share/qlc
macx:DATADIR       = Resources

# Documentation
win32:DOCSDIR      = Documents
unix:!macx:DOCSDIR = $$DATADIR/documents
macx:DOCSDIR       = $$DATADIR/Documents

# Input profiles
win32:INPUTPROFILEDIR          = InputProfiles
unix:!macx:INPUTPROFILEDIR     = $$DATADIR/inputprofiles
macx:INPUTPROFILEDIR           = $$DATADIR/InputProfiles

# User input profiles
win32:USERINPUTPROFILEDIR      = QLC/InputProfiles
unix:!macx:USERINPUTPROFILEDIR = .qlc/inputprofiles
macx:USERINPUTPROFILEDIR       = "Library/Application Support/QLC/InputProfiles"

# Fixtures
win32:FIXTUREDIR          = Fixtures
unix:!macx:FIXTUREDIR     = $$DATADIR/fixtures
macx:FIXTUREDIR           = $$DATADIR/Fixtures

# User fixtures
win32:USERFIXTUREDIR      = QLC/Fixtures
unix:!macx:USERFIXTUREDIR = .qlc/fixtures
macx:USERFIXTUREDIR       = "Library/Application Support/QLC/Fixtures"

# Plugins
win32:PLUGINDIR      = Plugins
unix:!macx:PLUGINDIR = $$LIBSDIR/qt4/plugins/qlc
macx:PLUGINDIR       = PlugIns

# Input Plugins
win32:INPUTPLUGINDIR      = $$PLUGINDIR/Input
unix:!macx:INPUTPLUGINDIR = $$PLUGINDIR/input
macx:INPUTPLUGINDIR       = $$PLUGINDIR/Input

# Output Plugins
win32:OUTPUTPLUGINDIR      = $$PLUGINDIR/Output
unix:!macx:OUTPUTPLUGINDIR = $$PLUGINDIR/output
macx:OUTPUTPLUGINDIR       = $$PLUGINDIR/Output

# Translations
win32:TRANSLATIONDIR      =
unix:!macx:TRANSLATIONDIR = $$DATADIR/translations
macx:TRANSLATIONDIR       = $$DATADIR/Translations
