include(../../../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_dmxusbout

QT             += core xml gui testlib
INCLUDEPATH    += ../../src
INCLUDEPATH    += ../../../../interfaces
QMAKE_CXXFLAGS += $$system(pkg-config --cflags libftdi)

unix:!macx {
    QT      += dbus
    DEFINES += DBUS_ENABLED
}

!CONFIG(coverage) {
    # While not measuring coverage mock objects need to be in the test binary
    # instead of the actual plugin.
    SOURCES += ../ftdimock/ftdimock.cpp
    HEADERS += ../ftdimock/ftdimock.h
    unix:!macx:SOURCES += ../ftdimock/dbusmock.cpp

    # Since Mac can't override ftdi functions as easily as normal unices, don't
    # link directly to the plugin which already contains linking to libftdi.
    # Instead, compile the sources under test into this test binary, which
    # contains only a mock implementation of ftdi. That's actually all we need.
    SOURCES += ../../src/enttecdmxusbout.cpp \
               ../../src/enttecdmxusbopen.cpp \
               ../../src/enttecdmxusbpro.cpp
    HEADERS += ../../src/enttecdmxusbout.h \
               ../../src/enttecdmxusbopen.h \
               ../../src/enttecdmxusbpro.h
} else {
    # While measuring coverage, ftdimock is linked to the plugin directly so
    # we can just link this test with the plugin.
    LIBS += -L../../src -lenttecdmxusbout
}

HEADERS += enttecdmxusbout_test.h
SOURCES += enttecdmxusbout_test.cpp
