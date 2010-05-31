LIBQTCORE_DIR = QtCore.framework/Versions/4
LIBQTCORE_FILE = QtCore
LIBQTCORE_FILEPATH = $$LIBQTCORE_DIR/$$LIBQTCORE_FILE

LIBQTCORE_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTCORE_FILEPATH \
			@executable_path/../Frameworks/$$LIBQTCORE_DIR/$$LIBQTCORE_FILE

contains(QT, core) {
        isEmpty(nametool.commands) {
        } else {
                nametool.commands += "&&"
        }

        nametool.commands += $$LIBQTCORE_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTCORE.path   = $$INSTALLROOT/$$LIBSDIR
LIBQTCORE.files += /Library/Frameworks/$$LIBQTCORE_FILEPATH
