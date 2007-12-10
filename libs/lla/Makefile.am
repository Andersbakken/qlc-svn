lib_LTLIBRARIES = libllaout.la

INCLUDES  = @QT_CFLAGS@ @LIBLLA_CFLAGS@ -I$(top_srcdir)/libs
INCLUDES += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
INCLUDES += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
INCLUDES += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
INCLUDES += -DPLUGINS=\"$(PLUGINS)\"

libllaout_la_LIBADD = @QT_LIBS@ @LIBLLA_LIBS@

noinst_HEADERS = \
	uic_configurellaout.h \
	\
	configurellaout.h \
	llaout.h

libllaout_la_SOURCES = \
	uic_configurellaout.cpp \
	\
	moc_uic_configurellaout.cpp \
	\
	llaout.cpp \
	configurellaout.cpp \
	\
	moc_llaout.cpp \
	moc_configurellaout.cpp

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
