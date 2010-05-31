LIBFTDI_DIR = $$system("pkg-config --variable libdir libftdi")

LIBFTDI_FILE = libftdi.1.dylib

LIBFTDI_FILEPATH = $$LIBFTDI_DIR/$$LIBFTDI_FILE
LIBUSB1_FILEPATH = $$LIBFTDI_DIR/$$LIBFTDI_FILE

LIBFTDI_INSTALL_NAME_TOOL = install_name_tool -change $$LIBFTDI_FILEPATH \
				@executable_path/../Frameworks/$$LIBFTDI_FILE

contains(PKGCONFIG, libftdi) {
        isEmpty(nametool.commands) {
        } else {
                nametool.commands += "&&" 
        }

        nametool.commands += $$LIBFTDI_INSTALL_NAME_TOOL $$OUTFILE
}

LIBFTDI.path     = $$INSTALLROOT/$$LIBSDIR
LIBFTDI.files   += $$LIBFTDI_FILEPATH

