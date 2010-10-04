TEMPLATE            = subdirs
unix:!macx:SUBDIRS += alsa
macx:SUBDIRS       += macx
win32:SUBDIRS      += win32
