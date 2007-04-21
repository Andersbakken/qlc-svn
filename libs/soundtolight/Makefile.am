lib_LTLIBRARIES = libsoundtolight.la

INCLUDES = @QT_CFLAGS@ -I$(top_srcdir)/libs
INCLUDES += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
INCLUDES += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
INCLUDES += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
INCLUDES += -DPLUGINS=\"$(PLUGINS)\"

libsoundtolight_la_LIBADD = @QT_LIBS@

noinst_HEADERS = \
	uic_configuresoundtolight.h \
	\
	configuresoundtolight.h \
	soundtolight.h

libsoundtolight_la_SOURCES = \
	uic_configuresoundtolight.cpp \
	\
	moc_uic_configuresoundtolight.cpp \
	\
	soundtolight.cpp \
	configuresoundtolight.cpp \
	\
	moc_soundtolight.cpp \
	moc_configuresoundtolight.cpp

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
