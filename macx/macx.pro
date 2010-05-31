include(../variables.pri)

TEMPLATE = subdirs

include(libusb-nametool.pri)
include(libftdi-nametool.pri)
include(libqtgui-nametool.pri)
include(libqtxml-nametool.pri)
include(libqtcore-nametool.pri)
include(libqtnetwork-nametool.pri)

INSTALLS += LIBUSB
INSTALLS += LIBFTDI
INSTALLS += LIBQTGUI
INSTALLS += LIBQTXML
INSTALLS += LIBQTCORE
INSTALLS += LIBQTNETWORK
