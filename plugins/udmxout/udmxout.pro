TEMPLATE            = subdirs
win32:SUBDIRS      += win32
macx:SUBDIRS       += macx
unix:!macx:SUBDIRS += unix
