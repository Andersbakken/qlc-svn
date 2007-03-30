lib_LTLIBRARIES = libllaout.la

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\" \
	$(LIBLLA_CFLAGS)

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

libllaout_la_LIBADD = @QT_LIBS@ $(LIBLLA_LDFLAGS)

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
