include(../variables.pri)

TEMPLATE = subdirs

docs.path = $$INSTALLROOT/$$DOCSDIR/html
docs.files = \
             addeditfixtures.html \
             addvcbuttonmatrix.html \
             busmanager.html \
             capabilityeditor.html \
             capabilitywizard.html \
             channeleditor.html \
             chasereditor.html \
             collectioneditor.html \
             commandlineparameters.html \
             concept.html \
             efxeditor.html \
             fixturedefinitioneditor.html \
             fixtureeditor.html \
             fixturemanager.html \
             fixturemonitor.html \
             functionmanager.html \
             functionwizard.html \
             howto-add-fixtures.html \
             howto-input-devices.html \
             howto-input-mapping.html \
             howto-output-mapping.html \
             index.html \
             modeeditor.html \
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
