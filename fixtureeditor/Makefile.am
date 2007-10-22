bin_PROGRAMS = qlcfixtureeditor

qlcfixtureeditor_CXXFLAGS  = @QT_CFLAGS@ -I$(top_srcdir)/libs
qlcfixtureeditor_CXXFLAGS += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
qlcfixtureeditor_CXXFLAGS += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
qlcfixtureeditor_CXXFLAGS += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
qlcfixtureeditor_CXXFLAGS += -DPLUGINS=\"$(PLUGINS)\"

qlcfixtureeditor_LDADD  = @QT_LIBS@
qlcfixtureeditor_LDADD += $(top_srcdir)/libs/common/libqlccommon.la

noinst_HEADERS = \
	src/uic_editchannel.h \
	src/uic_editmode.h \
	src/uic_fixtureeditor.h \
	\
	src/aboutbox.h \
	src/app.h \
	src/editchannel.h \
	src/editcapability.h \
	src/editmode.h \
	src/fixtureeditor.h

qlcfixtureeditor_SOURCES = \
	src/uic_editchannel.cpp \
	src/uic_editcapability.cpp \
	src/uic_editmode.cpp \
	src/uic_fixtureeditor.cpp \
	\
	src/moc_uic_editchannel.cpp \
	src/moc_uic_editcapability.cpp \
	src/moc_uic_editmode.cpp \
	src/moc_uic_fixtureeditor.cpp \
	\
	src/aboutbox.cpp \
	src/app.cpp \
	src/main.cpp \
	src/editchannel.cpp \
	src/editcapability.cpp \
	src/editmode.cpp \
	src/fixtureeditor.cpp \
	\
	src/moc_aboutbox.cpp \
	src/moc_app.cpp \
	src/moc_editchannel.cpp \
	src/moc_editcapability.cpp \
	src/moc_editmode.cpp \
	src/moc_fixtureeditor.cpp

# MOCS
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
