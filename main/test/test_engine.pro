include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_engine

DEFINES += QLC_UNIT_TEST
CONFIG 	+= warn_on qtestlib
CONFIG 	-= app_bundle
QT 	+= xml

INCLUDEPATH += ..
INCLUDEPATH += ../../libs/common
DEPENDPATH  += ..

# Windows needs even the test to be installed (because of libqlccommon.dll)
target.path = $$BINDIR
INSTALLS += target

unix:LIBS += ../../libs/common/libqlccommon.a
win32:{
        # Windows is so fucking retarded that you can't link these statically
        release:LIBS    += -L../libs/common/release -lqlccommon
        debug:LIBS      += -L../libs/common/debug -lqlccommon
}

ENGINEHEADERS += ../bus.h \
		 ../fixture.h \
		 ../doc.h \
		 ../function.h \
		 ../scene.h \
		 ../chaser.h \
		 ../collection.h \
		 ../efx.h \
		 ../efxfixture.h \
		 ../mastertimer.h \
		 ../outputmap.h \
		 ../outputpatch.h \
		 ../inputmap.h \
		 ../inputpatch.h \
		 ../dummyoutplugin.h

ENGINESOURCES += ../bus.cpp \
		 ../fixture.cpp \
		 ../doc.cpp \
		 ../function.cpp \
		 ../scene.cpp \
		 ../chaser.cpp \
		 ../collection.cpp \
		 ../efx.cpp \
		 ../efxfixture.cpp \
		 ../mastertimer.cpp \
		 ../outputmap.cpp \
		 ../outputpatch.cpp \
		 ../inputmap.cpp \
		 ../inputpatch.cpp \
		 ../dummyoutplugin.cpp

STUBHEADERS += mastertimer_stub.h \
		scene_stub.h \
		outputplugin_stub.h \
		inputplugin_stub.h \
		outputmap_stub.h \
		function_stub.h

STUBSOURCES += mastertimer_stub.cpp \
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
	  $$ENGINEHEADERS \
	  $$COMMONHEADERS \
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
	  $$ENGINESOURCES \
	  $$COMMONSOURCES \
	  $$STUBSOURCES

