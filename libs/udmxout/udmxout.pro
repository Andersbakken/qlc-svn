TEMPLATE 		= subdirs

win32:subdirs 		+= win32
macx:subdirs 		+= macx
unix:!macx:subdirs 	+= unix
