include(../variables.pri)

TEMPLATE = subdirs

docs.path = $$INSTALLROOT/$$DOCSDIR/html
docs.files = \
             addeditfixtures.html \
             addvcbuttonmatrix.html \
             busmanager.html \
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
             selectinputchannel.html \
             tutorial.html \
             tutorial.png \
             vcbutton.html \
             vcbuttonmatrix.html \
             vccuelist.html \
             vcframe.html \
             vclabel.html \
             vcsoloframe.html \
             vcslider.html \
             vcslidermatrix.html \
             vcstylingplacement.html \
             vcxypad.html \
             virtualconsole.html
INSTALLS += docs
