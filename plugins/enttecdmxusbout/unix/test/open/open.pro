include(../../../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_dmxusbopen

QT             += core xml gui testlib
INCLUDEPATH    += ../../src
INCLUDEPATH    += ../ftdimock
QMAKE_CXXFLAGS += $$system(pkg-config --cflags libftdi)

!CONFIG(coverage) {
    # While not measuring coverage ftdimock needs to be in the test binary
    # instead of the actual plugin.     
    SOURCES += ../ftdimock/ftdimock.cpp

    # Since Mac can't override ftdi functions as easily as normal unices, don't
    # link directly to the plugin which already contains linking to libftdi.
    # Instead, compile the sources under test into this test binary, which
    # contains only a mock implementation of ftdi. That's actually all we need.
    SOURCES += ../../src/enttecdmxusbopen.cpp
    HEADERS += ../../src/enttecdmxusbopen.h  
} else {
    # While measuring coverage, ftdimock is linked to the plugin directly so
    # we can just link this test with the plugin.
    LIBS += -L../../src -lenttecdmxusbout
}

HEADERS += enttecdmxusbopen_test.h
SOURCES += enttecdmxusbopen_test.cpp
