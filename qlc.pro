include(variables.pri)

TEMPLATE	= subdirs

SUBDIRS		= libs
SUBDIRS		+= main
SUBDIRS		+= fixtures
SUBDIRS		+= inputprofiles
SUBDIRS		+= fixtureeditor
SUBDIRS		+= etc
SUBDIRS		+= docs
SUBDIRS		+= main/test

DEBIAN_CLEAN	+= debian/*.substvars debian/*.log debian/*.debhelper
DEBIAN_CLEAN	+= debian/files debian/dirs
QMAKE_CLEAN	+= $$DEBIAN_CLEAN
QMAKE_DISTCLEAN += $$DEBIAN_CLEAN

unittests.target = test
unittests.commands += cd libs/common/test ; ./test_common ; \
                      cd ../../ewinginput/test ; ./test_ewing ; \
                      cd ../../../main/test ; ./test_engine
QMAKE_EXTRA_TARGETS += unittests
