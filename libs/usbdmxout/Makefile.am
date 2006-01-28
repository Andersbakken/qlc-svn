lib_LTLIBRARIES = libusbdmxout.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\"

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

libusbdmxout_la_LIBADD = @QT_LIBS@

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
