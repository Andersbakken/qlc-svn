include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = outputpluginstub

CONFIG      += plugin
INCLUDEPATH += ../../plugins/interfaces

HEADERS += outputpluginstub.h \
           ../../plugins/interfaces/qlcoutplugin.h
SOURCES += outputpluginstub.cpp
