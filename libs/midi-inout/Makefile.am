lib_LTLIBRARIES = libmidiinout.la

INCLUDES  = @QT_CFLAGS@ -I$(top_srcdir)/libs
INCLUDES += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
INCLUDES += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
INCLUDES += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
INCLUDES += -DPLUGINS=\"$(PLUGINS)\"

libmidiinout_la_LIBADD = @QT_LIBS@

noinst_HEADERS = \
	uic_configuremidi-inout.h \
	\
	configuremidi-inout.h \
	midi-inout.h

libmidiinout_la_SOURCES = \
	uic_configuremidi-inout.cpp \
	\
	moc_uic_configuremidi-inout.cpp \
	\
	midi-inout.cpp \
	configuremidi-inout.cpp \
	\
	moc_midi-inout.cpp \
	moc_configuremidi-inout.cpp

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
