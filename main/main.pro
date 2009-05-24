include(../variables.pri)

TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc

CONFIG          += qt
QT 		+= xml

TRANSLATIONS	= qlc_gb.ts \
		  qlc_es.ts

INCLUDEPATH 	+= . ../libs/
unix:LIBS 	+= -L../libs/common/ -lqlccommon
win32:{
	release:LIBS 	+= -L../libs/common/release/ -lqlccommon
	debug:LIBS 	+= -L../libs/common/debug/ -lqlccommon
}

# MAC Icon (TODO: Move under ../etc)
macx:ICON	= ../gfx/qlc.icns
macx:QMAKE_INFO_PLIST = ./Info.plist

# Bug in QT means it classifies directories as files
macx:QMAKE_DEL_FILE = rm -Rf
macx:QMAKE_CLEAN += qlc.app/

# Installation
target.path	= $$BINDIR
INSTALLS	+= target

# Resources
RESOURCES 	+= main.qrc
win32:RC_FILE	= main.rc

# Sources
HEADERS += aboutbox.h \
           addfixture.h \
           app.h \
           assignhotkey.h \
           bus.h \
           busmanager.h \
           chaser.h \
           chasereditor.h \
           collection.h \
           collectioneditor.h \
           consolechannel.h \
           doc.h \
           dummyoutplugin.h \
           efx.h \
           efxeditor.h \
           efxfixture.h \
           fixture.h \
           fixtureconsole.h \
           fixturemanager.h \
           fixtureproperties.h \
           fixtureselection.h \
           function.h \
           functionmanager.h \
           functionselection.h \
           generatorarea.h \
	   inputchanneleditor.h \
	   inputprofileeditor.h \
           inputmanager.h \
           inputmap.h \
	   inputpatch.h \
           inputpatcheditor.h \
           mastertimer.h \
           monitor.h \
           monitorfixture.h \
	   monitorlayout.h \
           outputmanager.h \
           outputmap.h \
	   outputpatch.h \
           outputpatcheditor.h \
           scene.h \
           sceneeditor.h \
           selectinputchannel.h \
           vcbutton.h \
           vcbuttonproperties.h \
           vccuelist.h \
           vccuelistproperties.h \
           vcdockarea.h \
           vcdockslider.h \
           vcframe.h \
           vclabel.h \
           vcproperties.h \
           vcslider.h \
           vcsliderproperties.h \
           vcwidget.h \
           vcxypad.h \
 	   vcxypadfixture.h \
 	   vcxypadfixtureeditor.h \
           vcxypadproperties.h \
           virtualconsole.h

FORMS += aboutbox.ui \
	 addfixture.ui \
         assignhotkey.ui \
         chasereditor.ui \
         collectioneditor.ui \
         efxeditor.ui \
         fixtureproperties.ui \
         fixtureselection.ui \
         functionselection.ui \
	 inputchanneleditor.ui \
	 inputprofileeditor.ui \
         inputpatcheditor.ui \
         outputpatcheditor.ui \
         sceneeditor.ui \
         selectinputchannel.ui \
         vcbuttonproperties.ui \
         vccuelistproperties.ui \
         vcdockslider.ui \
         vcproperties.ui \
         vcsliderproperties.ui \
         vcxypadfixtureeditor.ui \
         vcxypadproperties.ui

SOURCES += aboutbox.cpp \
           addfixture.cpp \
           app.cpp \
           assignhotkey.cpp \
           bus.cpp \
           busmanager.cpp \
           chaser.cpp \
           chasereditor.cpp \
           collection.cpp \
           collectioneditor.cpp \
           consolechannel.cpp \
           doc.cpp \
           dummyoutplugin.cpp \
           efx.cpp \
           efxeditor.cpp \
           efxfixture.cpp \
           fixture.cpp \
           fixtureconsole.cpp \
           fixturemanager.cpp \
           fixtureproperties.cpp \
           fixtureselection.cpp \
           function.cpp \
           functionmanager.cpp \
           functionselection.cpp \
           generatorarea.cpp \
	   inputchanneleditor.cpp \
	   inputprofileeditor.cpp \
           inputmanager.cpp \
           inputmap.cpp \
	   inputpatch.cpp \
           inputpatcheditor.cpp \
           main.cpp \
           mastertimer.cpp \
           monitor.cpp \
           monitorfixture.cpp \
	   monitorlayout.cpp \
           outputmanager.cpp \
           outputmap.cpp \
	   outputpatch.cpp \
           outputpatcheditor.cpp \
           scene.cpp \
           sceneeditor.cpp \
           selectinputchannel.cpp \
           vcbutton.cpp \
           vcbuttonproperties.cpp \
           vccuelist.cpp \
           vccuelistproperties.cpp \
           vcdockarea.cpp \
           vcdockslider.cpp \
           vcframe.cpp \
           vclabel.cpp \
           vcproperties.cpp \
           vcslider.cpp \
           vcsliderproperties.cpp \
           vcwidget.cpp \
           vcxypad.cpp \
           vcxypadfixture.cpp \
           vcxypadfixtureeditor.cpp \
           vcxypadproperties.cpp \
           virtualconsole.cpp

macx:QMAKE_POST_LINK = ./libupdate.sh
