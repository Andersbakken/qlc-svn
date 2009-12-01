TEMPLATE = app
LANGUAGE = C++
TARGET   = test

CONFIG	+= warn_on
CONFIG	-= app_bundle
QT 	+= testlib xml
DEFINES += QLC_UNIT_TEST

INCLUDEPATH += ../ ../../

HEADERS += qlcphysical_test.h \
	   ../qlcphysical.h \
	   qlcfixturemode_test.h \
	   ../qlcfixturemode.h \
	   qlcchannel_test.h \
	   ../qlcchannel.h \
	   qlccapability_test.h \
	   ../qlccapability.h \
	   qlcfixturedef_test.h \
	   ../qlcfixturedef.h \
	   qlcfixturedefcache_test.h \
	   ../qlcfixturedefcache.h \
	   \
	   qlcinputchannel_test.h \
	   ../qlcinputchannel.h \
	   qlcinputprofile_test.h \
	   ../qlcinputprofile.h \
	   \
	   qlcwidgetproperties_test.h \
	   ../qlcwidgetproperties.h \
	   \
	   ../qlcfile.h \
	   \
	   qlcmacros_test.h

SOURCES += qlcphysical_test.cpp \
	   ../qlcphysical.cpp \
	   qlcfixturemode_test.cpp \
	   ../qlcfixturemode.cpp \
	   qlcchannel_test.cpp \
	   ../qlcchannel.cpp \
	   qlccapability_test.cpp \
	   ../qlccapability.cpp \
	   qlcfixturedef_test.cpp \
	   ../qlcfixturedef.cpp \
	   qlcfixturedefcache_test.cpp \
	   ../qlcfixturedefcache.cpp \
	   \
	   qlcinputchannel_test.cpp \
	   ../qlcinputchannel.cpp \
	   qlcinputprofile_test.cpp \
	   ../qlcinputprofile.cpp \
	   \
	   qlcwidgetproperties_test.cpp \
	   ../qlcwidgetproperties.cpp \
	   \
	   ../qlcfile.cpp \
	   \
	   qlcmacros_test.cpp

SOURCES += main.cpp
