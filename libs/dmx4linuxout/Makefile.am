lib_LTLIBRARIES = libdmx4linuxout.la

INCLUDES  = @QT_CFLAGS@ -I$(top_srcdir)/libs
INCLUDES += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
INCLUDES += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
INCLUDES += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
INCLUDES += -DPLUGINS=\"$(PLUGINS)\"

libdmx4linuxout_la_LIBADD = @QT_LIBS@

noinst_HEADERS = \
	uic_configuredmx4linuxout.h \
	\
	configuredmx4linuxout.h \
	dmx4linuxout.h

libdmx4linuxout_la_SOURCES = \
	uic_configuredmx4linuxout.cpp \
	\
	moc_uic_configuredmx4linuxout.cpp \
	\
	dmx4linuxout.cpp \
	configuredmx4linuxout.cpp \
	\
	moc_dmx4linuxout.cpp \
	moc_configuredmx4linuxout.cpp

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
