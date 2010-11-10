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
             howto-add-fixtures.html \
             howto-input-devices.html \
             howto-input-mapping.html \
             howto-output-mapping.html \
             index.html \
             sceneeditor.html \
             selectfunction.html \
             selectfixture.html \
             tutorial.html \
             tutorial.png
INSTALLS += docs

gfx.path  = $$INSTALLROOT/$$DOCSDIR/gfx
gfx.files = \
            ../gfx/attach.png \
            ../gfx/back.png \
            ../gfx/beam.png \
            ../gfx/blackout.png \
            ../gfx/bus.png \    
            ../gfx/button.png \
            ../gfx/chaser.png \
            ../gfx/check.png \
            ../gfx/clock.png \
            ../gfx/collection.png \
            ../gfx/color.png \
            ../gfx/configure.png \
            ../gfx/console.png \
            ../gfx/cuelist.png \
            ../gfx/design.png \
            ../gfx/detach.png \
            ../gfx/down.png \
            ../gfx/edit_add.png \
            ../gfx/editclear.png \
            ../gfx/editcopy.png \
            ../gfx/editcopyall.png \
            ../gfx/editcut.png \
            ../gfx/editdelete.png \
            ../gfx/editpaste.png \
            ../gfx/edit.png \
            ../gfx/edit_remove.png \
            ../gfx/efx.png \
            ../gfx/exit.png \
            ../gfx/fileclose.png \
            ../gfx/filenew.png \
            ../gfx/fileopen.png \
            ../gfx/filesaveas.png \
            ../gfx/filesave.png \
            ../gfx/fixture.png \
            ../gfx/fonts.png \
            ../gfx/forward.png \
            ../gfx/frame.png \
            ../gfx/frameraised.png \
            ../gfx/framesunken.png \
            ../gfx/function.png \
            ../gfx/global.png \
            ../gfx/gobo.png \
            ../gfx/help.png \
            ../gfx/image.png \
            ../gfx/input.png \
            ../gfx/intensity.png \
            ../gfx/key_bindings.png \
            ../gfx/keyboard.png \
            ../gfx/knob.png \
            ../gfx/label.png \
            ../gfx/maintenance.png \
            ../gfx/monitor.png \
            ../gfx/movinghead.png \
            ../gfx/operate.png \
            ../gfx/output.png \
            ../gfx/panic.png \
            ../gfx/pan.png \
            ../gfx/prism.png \
            ../gfx/qlc-fixtureeditor.png \
            ../gfx/qlc.png \
            ../gfx/scene.png \
            ../gfx/shutter.png \
            ../gfx/slider.png \
            ../gfx/speed.png \
            ../gfx/tilt.png \
            ../gfx/uncheck.png \
            ../gfx/undo.png \
            ../gfx/up.png \
            ../gfx/virtualconsole.png \
            ../gfx/wizard.png \
            ../gfx/xypad.png \
            ../gfx/xypad-point.png \
INSTALLS += gfx
