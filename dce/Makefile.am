bin_PROGRAMS = qlcfixtureeditor

qlcfixtureeditor_CXXFLAGS  = @QT_CFLAGS@ -I$(top_srcdir)/libs
qlcfixtureeditor_CXXFLAGS += -DFIXTURES=\"$(datadir)/$(FIXTURES)\"
qlcfixtureeditor_CXXFLAGS += -DPIXMAPS=\"$(datadir)/$(PIXMAPS)\"
qlcfixtureeditor_CXXFLAGS += -DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\"
qlcfixtureeditor_CXXFLAGS += -DPLUGINS=\"$(PLUGINS)\"

qlcfixtureeditor_LDADD  = @QT_LIBS@
qlcfixtureeditor_LDADD += $(top_srcdir)/libs/common/libqlccommon.la

noinst_HEADERS = \
	src/uic_deviceclasseditor.h \
	src/uic_editchannel.h \
	src/uic_editpresetvalue.h \
	src/uic_editmode.h \
	\
	src/aboutbox.h \
	src/app.h \
	src/configkeys.h \
	src/deviceclasseditor.h \
	src/editchannel.h \
	src/editpresetvalue.h \
	src/editmode.h

qlcfixtureeditor_SOURCES = \
	src/uic_deviceclasseditor.cpp \
	src/uic_editchannel.cpp \
	src/uic_editpresetvalue.cpp \
	src/uic_editmode.cpp \
	\
	src/moc_uic_deviceclasseditor.cpp \
	src/moc_uic_editchannel.cpp \
	src/moc_uic_editpresetvalue.cpp \
	src/moc_uic_editmode.cpp \
	\
	src/aboutbox.cpp \
	src/app.cpp \
	src/main.cpp \
	src/deviceclasseditor.cpp \
	src/editchannel.cpp \
	src/editpresetvalue.cpp \
	src/editmode.cpp \
	\
	src/moc_aboutbox.cpp \
	src/moc_app.cpp \
	src/moc_deviceclasseditor.cpp \
	src/moc_editchannel.cpp \
	src/moc_editpresetvalue.cpp \
	src/moc_editmode.cpp

# MOCS
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
