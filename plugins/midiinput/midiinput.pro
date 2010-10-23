TEMPLATE           = subdirs
SUBDIRS           += common
win32:SUBDIRS     += win32
unix:!mac:SUBDIRS += alsa
mac:SUBDIRS       += macx
