bin_PROGRAMS = qlcfixtureeditor

INCLUDES = @QT_CFLAGS@ \
	-DFIXTURES=\"$(datadir)/$(FIXTURES)\" \
	-DPIXMAPS=\"$(datadir)/$(PIXMAPS)\" \
	-DDOCUMENTS=\"$(datadir)/$(DOCUMENTS)/$(PACKAGE)\" \
	-DPLUGINS=\"$(PLUGINS)\" \
	\
	-I$(top_srcdir)/libs

noinst_HEADERS = \
	src/uic_deviceclasseditor.h \
	src/uic_editpresetvalue.h \
	\
	src/aboutbox.h \
	src/app.h \
	src/deviceclasseditor.h \
	src/editpresetvalue.h

qlcfixtureeditor_SOURCES = \
	src/uic_deviceclasseditor.cpp \
	src/uic_editpresetvalue.cpp \
	\
	src/moc_uic_deviceclasseditor.cpp \
	src/moc_uic_editpresetvalue.cpp \
	\
	src/aboutbox.cpp \
	src/app.cpp \
	src/deviceclasseditor.cpp \
	src/editpresetvalue.cpp \
	src/main.cpp \
	\
	src/moc_aboutbox.cpp \
	src/moc_app.cpp \
	src/moc_deviceclasseditor.cpp \
	src/moc_editpresetvalue.cpp \
	\
	../main/src/capability.cpp \
	../main/src/deviceclass.cpp \
	../main/src/logicalchannel.cpp \
	\
	../main/src/moc_capability.cpp \
	../main/src/moc_deviceclass.cpp \
	../main/src/moc_logicalchannel.cpp

qlcfixtureeditor_LDADD = ../libs/common/libqlccommon.la @QT_LIBS@

# MOCS
moc_%.cpp: %.h
	$(MOC) $< -o $@

# UIC headers
uic_%.h: %.ui
	$(UIC) $< -o $@

# UIC sources
uic_%.cpp: uic_%.h
	$(UIC) -impl $< $*.ui -o $@
