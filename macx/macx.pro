include(../variables.pri)

TEMPLATE = subdirs
CONFIG  += ordered

include(libusb-nametool.pri)
include(libftdi-nametool.pri)
include(libqtgui-nametool.pri)
include(libqtxml-nametool.pri)
include(libqtcore-nametool.pri)
include(libqtnetwork-nametool.pri)

INSTALLS += LIBUSB
INSTALLS += LIBFTDI
INSTALLS += LIBQTGUI QTMENU LIBQTGUI_ID
INSTALLS += LIBQTXML LIBQTXML_ID
INSTALLS += LIBQTCORE LIBQTCORE_ID
INSTALLS += LIBQTNETWORK LIBQTNETWORK_ID

# QtGui, QtXml and QtNetwork depend on QtCore. Do this AFTER installing the
# libraries into the bundle
qtnametool.path = $$INSTALLROOT
qtnametool.commands = $$LIBQTCORE_INSTALL_NAME_TOOL \
	$$INSTALLROOT/Frameworks/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE
qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL \
	$$INSTALLROOT/Frameworks/$$LIBQTXML_DIR/$$LIBQTXML_FILE
qtnametool.commands += && $$LIBQTCORE_INSTALL_NAME_TOOL \
	$$INSTALLROOT/Frameworks/$$LIBQTXML_DIR/$$LIBQTXML_FILE

include(imageformats-nametool.pri)
INSTALLS += imageformats

INSTALLS += qtnametool

