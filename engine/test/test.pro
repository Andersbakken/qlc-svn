include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_engine

CONFIG  += qtestlib
QT      += xml
QTPLUGIN =

INCLUDEPATH += ../src
DEPENDPATH  += ../src
INCLUDEPATH += ../../plugins/interfaces
INCLUDEPATH += ../inputpluginstub ../outputpluginstub
QMAKE_LIBDIR += ../src
LIBS   += -lqlcengine

# Fixture metadata
HEADERS += qlcphysical_test.h \
           qlcfixturemode_test.h \
           qlcchannel_test.h \
           qlccapability_test.h \
           qlcfixturedef_test.h \
           qlcfixturedefcache_test.h \
           qlcinputchannel_test.h \
           qlcinputprofile_test.h \
           qlcmacros_test.h \
           qlcfile_test.h \
           qlci18n_test.h

# Engine
HEADERS += bus_test.h \
           chaserrunner_test.h \
           fadechannel_test.h \
           fixture_test.h \
           function_test.h \
           scene_test.h \
           scenevalue_test.h \
           chaser_test.h \
           collection_test.h \
           efx_test.h \
           efxfixture_test.h \
           universearray_test.h \
           outputpatch_test.h \
           inputpatch_test.h \
           outputmap_test.h \
           inputmap_test.h \
           mastertimer_test.h \
           doc_test.h \
           palettegenerator_test.h

# Interfaces
HEADERS += ../../plugins/interfaces/qlcoutplugin.h

# Stubs
HEADERS += mastertimer_stub.h \
           dmxsource_stub.h \
           scene_stub.h \
           outputmap_stub.h \
           function_stub.h

# Fixture metadata
SOURCES += qlcphysical_test.cpp \
           qlcfixturemode_test.cpp \
           qlcchannel_test.cpp \
           qlccapability_test.cpp \
           qlcfixturedef_test.cpp \
           qlcfixturedefcache_test.cpp \
           qlcinputchannel_test.cpp \
           qlcinputprofile_test.cpp \
           qlcmacros_test.cpp \
           qlcfile_test.cpp \
           qlci18n_test.cpp

# Engine
SOURCES += bus_test.cpp \
           chaserrunner_test.cpp \
           fadechannel_test.cpp \
           fixture_test.cpp \
           function_test.cpp \
           scene_test.cpp \
           scenevalue_test.cpp \
           chaser_test.cpp \
           collection_test.cpp \
           efx_test.cpp \
           efxfixture_test.cpp \
           universearray_test.cpp \
           outputpatch_test.cpp \
           inputpatch_test.cpp \
           outputmap_test.cpp \
           inputmap_test.cpp \
           mastertimer_test.cpp \
           doc_test.cpp \
           palettegenerator_test.cpp
    
# Stubs
SOURCES += mastertimer_stub.cpp \
           dmxsource_stub.cpp \
           scene_stub.cpp \
           outputmap_stub.cpp \
           function_stub.cpp

# Test main
SOURCES += main.cpp
