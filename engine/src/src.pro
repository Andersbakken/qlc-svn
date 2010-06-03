include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = qlcengine

CONFIG 		+= staticlib
CONFIG 		+= qt
macx:CONFIG     -= app_bundle
QT              += xml

INCLUDEPATH     += ../../libs/common

unix:LIBS       += ../../libs/common/libqlccommon.a
win32:{
        CONFIG(release, debug|release) LIBS += ../../libs/common/release/libqlccommon.a
        CONFIG(debug, debug|release) LIBS += ../../libs/common/debug/libqlccommon.a
}

HEADERS += bus.h \
           chaser.h \
           collection.h \
           doc.h \
           dummyoutplugin.h \
           efx.h \
           efxfixture.h \
           fixture.h \
           function.h \
           inputmap.h \
           inputpatch.h \
	   intensitygenerator.h \
           mastertimer.h \
           outputmap.h \
           outputpatch.h \
	   palettegenerator.h \
           scene.h

SOURCES += bus.cpp \
           chaser.cpp \
           collection.cpp \
           doc.cpp \
           dummyoutplugin.cpp \
           efx.cpp \
           efxfixture.cpp \
           fixture.cpp \
           function.cpp \
           inputmap.cpp \
           inputpatch.cpp \
	   intensitygenerator.cpp \
           mastertimer.cpp \
           outputmap.cpp \
           outputpatch.cpp \
	   palettegenerator.cpp \
           scene.cpp

