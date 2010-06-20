LIBQTGUI_DIR = QtGui.framework/Versions/4
LIBQTGUI_FILE = QtGui
LIBQTGUI_FILEPATH = $$LIBQTGUI_DIR/$$LIBQTGUI_FILE

LIBQTGUI_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTGUI_FILEPATH \
			@executable_path/../Frameworks/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE

contains(QT, gui) {
        isEmpty(nametool.commands) {
        } else {
                nametool.commands += "&&"
        }

        nametool.commands += $$LIBQTGUI_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTGUI.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTGUI_DIR
LIBQTGUI.files += /Library/Frameworks/$$LIBQTGUI_FILEPATH

QTMENU.path   = $$INSTALLROOT/$$DATADIR
QTMENU.files += /Library/Frameworks/$$LIBQTGUI_DIR/Resources/*

LIBQTGUI_INSTALL_NAME_TOOL_ID = install_name_tool -id @executable_path/../Frameworks/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE \
			$$INSTALLROOT/Frameworks/$$LIBQTGUI_DIR/$$LIBQTGUI_FILE
LIBQTGUI_ID.path   = $$INSTALLROOT/$$LIBSDIR/$$LIBQTGUI_DIR
LIBQTGUI_ID.commands = $$LIBQTGUI_INSTALL_NAME_TOOL_ID
