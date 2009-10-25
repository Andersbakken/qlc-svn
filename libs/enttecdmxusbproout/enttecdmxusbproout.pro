TEMPLATE		= subdirs

win32:SUBDIRS		+= win32
unix:!macx:SUBDIRS	+= unix
macx:SUBDIRS		+= macx
