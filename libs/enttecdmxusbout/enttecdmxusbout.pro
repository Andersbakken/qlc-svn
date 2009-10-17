TEMPLATE 		= subdirs

unix:!macx:SUBDIRS 	+= unix
macx:SUBDIRS 		+= macx
win32:SUBDIRS		+= win32
