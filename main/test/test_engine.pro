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

HEADERS = bus_test.h \
	  ../bus.h \
	  fixture_test.h \
	  ../fixture.h \
	  \
	  $$COMMONHEADERS

SOURCES = bus_test.cpp \
	  ../bus.cpp \
	  fixture_test.cpp \
	  ../fixture.cpp \
	  \
	  test_engine.cpp \
	  \
	  $$COMMONSOURCES
