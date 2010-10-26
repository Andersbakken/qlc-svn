include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = inputpluginstub

CONFIG      += plugin
INCLUDEPATH += ../../plugins/interfaces

HEADERS += inputpluginstub.h \
           ../../plugins/interfaces/qlcinplugin.h
SOURCES += inputpluginstub.cpp
