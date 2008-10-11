TEMPLATE	= subdirs

SUBDIRS		= libs
SUBDIRS		+= main
SUBDIRS		+= fixtures
SUBDIRS		+= inputtemplates
SUBDIRS		+= fixtureeditor
SUBDIRS		+= etc

macx {
	system(rm -Rf main/qlc.app/Contents/docs)
	system(mkdir -p main/qlc.app/Contents/)
	system(cp -r docs main/qlc.app/Contents/)
	system(rm -Rf fixtureeditor/qlc-fixtureeditor.app/Contents/docs)
	system(mkdir -p fixtureeditor/qlc-fixtureeditor.app/Contents/)
	system(cp -r docs fixtureeditor/qlc-fixtureeditor.app/Contents/)
}
