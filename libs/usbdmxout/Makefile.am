lib_LTLIBRARIES = libusbdmxout.la

INCLUDES  = @QT_CFLAGS@ -I$(top_srcdir)/libs
INCLUDES += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
INCLUDES += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
INCLUDES += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
INCLUDES += -DPLUGINS=\"$(PLUGINS)\"

libusbdmxout_la_LIBADD = @QT_LIBS@

noinst_HEADERS = \
	uic_configureusbdmxout.h \
	\
	configureusbdmxout.h \
	usbdmxout.h

libusbdmxout_la_SOURCES = \
	uic_configureusbdmxout.cpp \
	\
	moc_uic_configureusbdmxout.cpp \
	\
	usbdmxout.cpp \
	configureusbdmxout.cpp \
	\
	moc_usbdmxout.cpp \
	moc_configureusbdmxout.cpp

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
