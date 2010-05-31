LIBQTNETWORK_DIR = QtNetwork.framework/Versions/4
LIBQTNETWORK_FILE = QtNetwork
LIBQTNETWORK_FILEPATH = $$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE

LIBQTNETWORK_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTNETWORK_FILEPATH \
		@executable_path/../Frameworks/$$LIBQTNETWORK_DIR/$$LIBQTNETWORK_FILE

contains(QT, network) {
        isEmpty(nametool.commands) {
        } else {
                nametool.commands += "&&"
        }

        nametool.commands += $$LIBQTNETWORK_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTNETWORK.path   = $$INSTALLROOT/$$LIBSDIR
LIBQTNETWORK.files += /Library/Frameworks/$$LIBQTNETWORK_FILEPATH
