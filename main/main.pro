TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc
VERSION 	= 3.0

CONFIG          += qt warn_on release
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
unix:LIBS 	+= -L../libs/common/ -lqlccommon
win32:LIBS 	+= -L../libs/common/release/ -lqlccommon

unix:target.path = /usr/bin
win32:target.path = C:\QLC
INSTALLS	+= target

RESOURCES 	+= main.qrc
win32:RC_FILE	= main.rc

# Sources
HEADERS += aboutbox.h \
           addfixture.h \
           app.h \
	   appdirectories.h \
           assignhotkey.h \
           bus.h \
           busproperties.h \
           chaser.h \
           chasereditor.h \
           collection.h \
           collectioneditor.h \
           consolechannel.h \
           dmxmap.h \
           dmxmapeditor.h \
           dmxpatcheditor.h \
           doc.h \
           dummyoutplugin.h \
           efx.h \
           efxeditor.h \
           efxfixture.h \
           eventbuffer.h \
           fixture.h \
           fixtureconsole.h \
           fixturelist.h \
           fixturemanager.h \
           fixtureproperties.h \
           fixtureselection.h \
           function.h \
           functionconsumer.h \
           functionmanager.h \
           functionselection.h \
           generatorarea.h \
           inputmap.h \
           keybind.h \
           monitor.h \
           pluginmanager.h \
           scene.h \
           sceneeditor.h \
           vcbutton.h \
           vcbuttonproperties.h \
           vccuelist.h \
           vccuelistproperties.h \
           vcdockarea.h \
           vcdockslider.h \
           vcframe.h \
           vcframeproperties.h \
           vclabel.h \
           vcslider.h \
           vcsliderproperties.h \
           vcwidget.h \
           vcxypad.h \
           vcxypadproperties.h \
           virtualconsole.h \
           virtualconsoleproperties.h \
           xychannelunit.h

FORMS += addfixture.ui \
	 appdirectories.ui \
         assignhotkey.ui \
         busproperties.ui \
         chasereditor.ui \
         collectioneditor.ui \
         dmxmapeditor.ui \
         dmxpatcheditor.ui \
         efxeditor.ui \
         fixturelist.ui \
         fixtureproperties.ui \
         fixtureselection.ui \
         functionselection.ui \
         sceneeditor.ui \
         vcbuttonproperties.ui \
         vccuelistproperties.ui \
         vcdockslider.ui \
         vcframeproperties.ui \
         vcsliderproperties.ui \
         vcxypadproperties.ui \
         virtualconsoleproperties.ui

SOURCES += aboutbox.cpp \
           addfixture.cpp \
           app.cpp \
	   appdirectories.cpp \
           assignhotkey.cpp \
           bus.cpp \
           busproperties.cpp \
           chaser.cpp \
           chasereditor.cpp \
           collection.cpp \
           collectioneditor.cpp \
           consolechannel.cpp \
           dmxmap.cpp \
           dmxmapeditor.cpp \
           dmxpatcheditor.cpp \
           doc.cpp \
           dummyoutplugin.cpp \
           efx.cpp \
           efxeditor.cpp \
           efxfixture.cpp \
           eventbuffer.cpp \
           fixture.cpp \
           fixtureconsole.cpp \
           fixturelist.cpp \
           fixturemanager.cpp \
           fixtureproperties.cpp \
           fixtureselection.cpp \
           function.cpp \
           functionconsumer.cpp \
           functionmanager.cpp \
           functionselection.cpp \
           generatorarea.cpp \
           inputmap.cpp \
           keybind.cpp \
           main.cpp \
           monitor.cpp \
           pluginmanager.cpp \
           scene.cpp \
           sceneeditor.cpp \
           vcbutton.cpp \
           vcbuttonproperties.cpp \
           vccuelist.cpp \
           vccuelistproperties.cpp \
           vcdockarea.cpp \
           vcdockslider.cpp \
           vcframe.cpp \
           vcframeproperties.cpp \
           vclabel.cpp \
           vcslider.cpp \
           vcsliderproperties.cpp \
           vcwidget.cpp \
           vcxypad.cpp \
           vcxypadproperties.cpp \
           virtualconsole.cpp \
           virtualconsoleproperties.cpp \
           xychannelunit.cpp
