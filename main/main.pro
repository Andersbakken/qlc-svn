TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc

CONFIG          += qt warn_on release
QT 		+= xml

INCLUDEPATH 	+= . ../libs/
macx:LIBS 	+= -L./qlc.app/Contents/Frameworks -lqlccommon
unix:LIBS 	+= -L../libs/common/ -lqlccommon
win32:release {
	LIBS 	+= -L../libs/common/release/ -lqlccommon
}
win32:debug {
	LIBS 	+= -L../libs/common/debug/ -lqlccommon
}

macx:ICON	= ../gfx/qlc.icns
macx:QMAKE_INFO_PLIST = ./Info.plist

unix:target.path = /usr/bin
win32:target.path = $$(SystemDrive)/QLC
macx:target.path = /Applications
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
	   inputchanneleditor.h \
           inputmanager.h \
           inputmap.h \
	   inputpatch.h \
           inputpatcheditor.h \
	   inputtemplateeditor.h \
           keybind.h \
           monitor.h \
           outputmanager.h \
           outputmap.h \
	   outputpatch.h \
           outputpatcheditor.h \
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

FORMS += aboutbox.ui \
	 addfixture.ui \
	 appdirectories.ui \
         assignhotkey.ui \
         chasereditor.ui \
         collectioneditor.ui \
         efxeditor.ui \
         fixturelist.ui \
         fixtureproperties.ui \
         fixtureselection.ui \
         functionselection.ui \
	 inputchanneleditor.ui \
         inputpatcheditor.ui \
	 inputtemplateeditor.ui \
         outputmanager.ui \
         outputpatcheditor.ui \
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
	   inputchanneleditor.cpp \
           inputmanager.cpp \
           inputmap.cpp \
	   inputpatch.cpp \
           inputpatcheditor.cpp \
	   inputtemplateeditor.cpp \
           keybind.cpp \
           main.cpp \
           monitor.cpp \
           outputmanager.cpp \
           outputmap.cpp \
	   outputpatch.cpp \
           outputpatcheditor.cpp \
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

macx {
        system(mkdir -p qlc.app/Contents/Frameworks)
        LIBS_PATH = ../libs/common/libqlccommon*.dylib
        system(cp $$LIBS_PATH qlc.app/Contents/Frameworks)
	QMAKE_POST_LINK = install_name_tool -change libqlccommon.3.dylib @executable_path/../Frameworks/libqlccommon.3.dylib qlc.app/Contents/MacOS/qlc
}
