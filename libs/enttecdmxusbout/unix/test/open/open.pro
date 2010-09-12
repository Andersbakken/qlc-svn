TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbopen

INCLUDEPATH += ../../src

QT += core xml gui testlib
CONFIG += link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += libftdi

HEADERS += enttecdmxusbopen_test.h ../../src/enttecdmxusbopen.h
SOURCES += enttecdmxusbopen_test.cpp ../../src/enttecdmxusbopen.cpp

# Coverage
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
LIBS += -lgcov
QMAKE_CLEAN += *.gcno *.gcov
QMAKE_DISTCLEAN += *.gcno *.gcov
QMAKE_EXTRA_TARGETS += coverage
coverage.target = coverage
coverage.commands += gcov *.cpp *.h
