include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_engine
DEFINES += QLC_UNIT_TEST

CONFIG += warn_on qtestlib
QT += xml

INCLUDEPATH += ../ ../../libs/

target.path = $$BINDIR
INSTALLS += target

COMMONHEADERS += ../../libs/common/qlccapability.h \
		 ../../libs/common/qlcchannel.h \
		 ../../libs/common/qlcdocbrowser.h \
		 ../../libs/common/qlcfile.h \
		 ../../libs/common/qlcfixturedef.h \
		 ../../libs/common/qlcfixturedefcache.h \
		 ../../libs/common/qlcfixturemode.h \
		 ../../libs/common/qlcinputchannel.h \
		 ../../libs/common/qlcinputprofile.h \
		 ../../libs/common/qlcphysical.h \
		 ../../libs/common/qlctypes.h \
		 ../../libs/common/qlcwidgetproperties.h

COMMONSOURCES += ../../libs/common/qlccapability.cpp \
		 ../../libs/common/qlcchannel.cpp \
		 ../../libs/common/qlcdocbrowser.cpp \
		 ../../libs/common/qlcfile.cpp \
		 ../../libs/common/qlcfixturedef.cpp \
		 ../../libs/common/qlcfixturedefcache.cpp \
		 ../../libs/common/qlcfixturemode.cpp \
		 ../../libs/common/qlcinputchannel.cpp \
		 ../../libs/common/qlcinputprofile.cpp \
		 ../../libs/common/qlcphysical.cpp \
		 ../../libs/common/qlcwidgetproperties.cpp

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
		inputplugin_stub.h

STUBSOURCES += mastertimer_stub.cpp \
		scene_stub.cpp \
		outputplugin_stub.cpp \
		inputplugin_stub.cpp

HEADERS = bus_test.h \
	  fixture_test.h \
	  scene_test.h \
	  scenevalue_test.h \
	  scenechannel_test.h \
	  chaser_test.h \
	  collection_test.h \
	  efx_test.h \
	  efxfixture_test.h \
	  outputpatch_test.h \
	  inputpatch_test.h \
	  \
	  $$ENGINEHEADERS \
	  $$COMMONHEADERS \
	  $$STUBHEADERS

SOURCES = bus_test.cpp \
	  fixture_test.cpp \
	  scene_test.cpp \
	  scenevalue_test.cpp \
	  scenechannel_test.cpp \
	  chaser_test.cpp \
	  collection_test.cpp \
	  efx_test.cpp \
	  efxfixture_test.cpp \
	  outputpatch_test.cpp \
	  inputpatch_test.cpp \
	  \
	  test_engine.cpp \
	  \
	  $$ENGINESOURCES \
	  $$COMMONSOURCES \
	  $$STUBSOURCES
