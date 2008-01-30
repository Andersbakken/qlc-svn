lib_LTLIBRARIES = libhidinput.la

INCLUDES  = @QT_CFLAGS@ -I$(top_srcdir)/libs
INCLUDES += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
INCLUDES += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
INCLUDES += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
INCLUDES += -DPLUGINS=\"$(PLUGINS)\"

libhidinput_la_LIBADD = @QT_LIBS@

noinst_HEADERS = \
	uic_configurehidinput.h \
	\
	configurehidinput.h \
	hiddevice.h \
	hideventdevice.h \
	hidinput.h

libhidinput_la_SOURCES = \
	uic_configurehidinput.cpp \
	moc_uic_configurehidinput.cpp \
	\
	configurehidinput.cpp \
	hiddevice.cpp \
	hideventdevice.cpp \
	hidinput.cpp \
	\
	moc_configurehidinput.cpp \
	moc_hiddevice.cpp \
	moc_hideventdevice.cpp \
	moc_hidinput.cpp

libdir = $(PLUGINS)

moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
