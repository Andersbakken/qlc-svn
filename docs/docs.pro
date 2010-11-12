include(../variables.pri)

TEMPLATE = subdirs

docs.path = $$INSTALLROOT/$$DOCSDIR/html
docs.files = \
             addeditfixtures.html \
             chasereditor.html \
             collectioneditor.html \
             concept.html \
             efxeditor.html \
             fixturemanager.html \
             functionmanager.html \
             functionwizard.html \
             howto-add-fixtures.html \
             howto-input-devices.html \
             howto-input-mapping.html \
             howto-output-mapping.html \
             index.html \
             sceneeditor.html \
             selectfunction.html \
             selectfixture.html \
             tutorial.html \
             tutorial.png \
             vcframe.html \
             vclabel.html \
             vcsoloframe.html \
             vcstylingplacement.html \
             virtualconsole.html
INSTALLS += docs
