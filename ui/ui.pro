include(../variables.pri)

TEMPLATE 	= app
LANGUAGE 	= C++
TARGET 		= qlc

CONFIG          += qt
macx:CONFIG	-= app_bundle
QT 		+= xml

# macdeployqt checks dependencies only for the main app, forgetting QtNetwork,
# which is needed by the Enttec Wing plugin, so we need to add a bogus
# dependency here...
macx:QT		+= network

# Common
INCLUDEPATH 	+= ../libs/common
DEPENDPATH 	+= ../libs/common
unix:LIBS	+= ../libs/common/libqlccommon.a
win32:{
	CONFIG(release, debug|release) LIBS += ../libs/common/release/libqlccommon.a
	CONFIG(debug, debug|release) LIBS += ../libs/common/debug/libqlccommon.a
}

# Engine
INCLUDEPATH	+= ../engine/src
DEPENDPATH	+= ../engine/src
unix:LIBS	+= ../engine/src/libqlcengine.a

# Installation
target.path	= $$INSTALLROOT/$$BINDIR
INSTALLS	+= target

# Resources
RESOURCES 	+= main.qrc
win32:RC_FILE	= main.rc

# Sources
HEADERS += aboutbox.h \
           addfixture.h \
           app.h \
           assignhotkey.h \
           busmanager.h \
           chasereditor.h \
           collectioneditor.h \
           consolechannel.h \
           efxeditor.h \
           fixtureconsole.h \
           fixturemanager.h \
           fixtureselection.h \
           functionmanager.h \
           functionselection.h \
           generatorarea.h \
	   inputchanneleditor.h \
	   inputprofileeditor.h \
           inputmanager.h \
           inputpatcheditor.h \
           monitor.h \
           monitorfixture.h \
	   monitorlayout.h \
           outputmanager.h \
           outputpatcheditor.h \
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
           busmanager.cpp \
           chasereditor.cpp \
           collectioneditor.cpp \
           consolechannel.cpp \
           efxeditor.cpp \
           fixtureconsole.cpp \
           fixturemanager.cpp \
           fixtureselection.cpp \
           functionmanager.cpp \
           functionselection.cpp \
           generatorarea.cpp \
	   inputchanneleditor.cpp \
	   inputprofileeditor.cpp \
           inputmanager.cpp \
           inputpatcheditor.cpp \
           main.cpp \
           monitor.cpp \
           monitorfixture.cpp \
	   monitorlayout.cpp \
           outputmanager.cpp \
           outputpatcheditor.cpp \
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
