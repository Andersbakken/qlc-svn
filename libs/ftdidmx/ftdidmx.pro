TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= ftdidmx

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release
LIBS		+= -L. -lftd2xx.0.1.4

unix:target.path = /usr/lib/qlc/output
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

# Forms
FORMS += configureftdidmx.ui

# Headers
unix:HEADERS += configureftdidmx.h \
                ftdidmx.h \
		ftdidmxdevice.h

# Sources
unix:SOURCES += configureftdidmx.cpp \
                ftdidmx.cpp \
		ftdidmxdevice.cpp

macx {
	QMAKE_POST_LINK = cp libftd2xx.0.1.4.dylib ../../main/qlc.app/Contents/Plugins/output/
	QMAKE_POST_LINK += && install_name_tool -change /usr/local/lib/libftd2xx.0.1.4.dylib @executable_path/../Plugins/output/libftd2xx.0.1.4.dylib ../../main/qlc.app/Contents/Plugins/output/libftdidmx.dylib
}
