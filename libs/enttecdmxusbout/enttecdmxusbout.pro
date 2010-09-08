TEMPLATE 		= subdirs

unix:!macx:SUBDIRS 	+= unix
macx:SUBDIRS 		+= macx
unix:SUBDIRS		+= unix/test
win32:SUBDIRS		+= win32
