include(../variables.pri)

TEMPLATE = subdirs
TARGET = devices

devices.files += Behringer-BCF2000.qxi
devices.files += Novation-KS5.qxi
devices.files += Logitech-WingManAttack2.qxi

devices.path = $$INPUTDEVICEDIR
INSTALLS += devices
