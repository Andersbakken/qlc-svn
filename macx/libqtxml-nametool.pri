LIBQTXML_DIR = QtXml.framework/Versions/4
LIBQTXML_FILE = QtXml
LIBQTXML_FILEPATH = $$LIBQTXML_DIR/$$LIBQTXML_FILE

LIBQTXML_INSTALL_NAME_TOOL = install_name_tool -change $$LIBQTXML_FILEPATH \
			@executable_path/../Frameworks/$$LIBQTXML_DIR/$$LIBQTXML_FILE

contains(QT, xml) {
        isEmpty(nametool.commands) {
        } else {
                nametool.commands += "&&"
        }

        nametool.commands += $$LIBQTXML_INSTALL_NAME_TOOL $$OUTFILE
}

LIBQTXML.path   = $$INSTALLROOT/$$LIBSDIR
LIBQTXML.files += /Library/Frameworks/$$LIBQTXML_FILEPATH
