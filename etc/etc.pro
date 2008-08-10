TEMPLATE	= subdirs
TARGET		= icons

unix:desktop.path	= /usr/share/applications/
unix:desktop.files	+= qlc.desktop \
			   qlc-fixtureeditor.desktop

unix:icons.path		= /usr/share/pixmaps/
unix:icons.files	+= ../gfx/qlc.png \
			   ../gfx/qlc-fixtureeditor.png

unix:INSTALLS		+= icons desktop
