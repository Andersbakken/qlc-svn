TEMPLATE 		= subdirs

unix:!macx:SUBDIRS 	+= linux
macx:SUBDIRS 		+= macx
win32:SUBDIRS		+= win32
