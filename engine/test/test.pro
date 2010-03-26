include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_engine

QMAKE_CXXFLAGS -= -Werror
CONFIG 	+= qtestlib
CONFIG 	-= app_bundle
QT 	+= xml

INCLUDEPATH += ../src
INCLUDEPATH += ../../libs/common
DEPENDPATH  += ../src
LIBS        += ../src/libqlcengine.a

unix:LIBS	+= ../../libs/common/libqlccommon.a
win32:{
	CONFIG(release, debug|release) LIBS += ../../libs/common/release/libqlccommon.a
	CONFIG(debug, debug|release) LIBS += ../../libs/common/debug/libqlccommon.a
}

STUBHEADERS += mastertimer_stub.h \
		dmxsource_stub.h \
		scene_stub.h \
		outputplugin_stub.h \
		inputplugin_stub.h \
		outputmap_stub.h \
		function_stub.h

STUBSOURCES += mastertimer_stub.cpp \
		dmxsource_stub.cpp \
		scene_stub.cpp \
		outputplugin_stub.cpp \
		inputplugin_stub.cpp \
		outputmap_stub.cpp \
		function_stub.cpp

HEADERS = bus_test.h \
	  fixture_test.h \
	  function_test.h \
	  scene_test.h \
	  scenevalue_test.h \
	  scenechannel_test.h \
	  chaser_test.h \
	  collection_test.h \
	  efx_test.h \
	  efxfixture_test.h \
	  outputpatch_test.h \
	  inputpatch_test.h \
	  outputmap_test.h \
	  inputmap_test.h \
	  mastertimer_test.h \
	  doc_test.h \
	  \
	  $$STUBHEADERS

SOURCES = bus_test.cpp \
	  fixture_test.cpp \
	  function_test.cpp \
	  scene_test.cpp \
	  scenevalue_test.cpp \
	  scenechannel_test.cpp \
	  chaser_test.cpp \
	  collection_test.cpp \
	  efx_test.cpp \
	  efxfixture_test.cpp \
	  outputpatch_test.cpp \
	  inputpatch_test.cpp \
	  outputmap_test.cpp \
	  inputmap_test.cpp \
	  mastertimer_test.cpp \
	  doc_test.cpp \
	  \
	  test_engine.cpp \
	  \
	  $$STUBSOURCES

