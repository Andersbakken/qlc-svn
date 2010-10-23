#!/bin/bash

#############################################################################
# Engine tests
#############################################################################
pushd .
cd engine/test
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_engine
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Engine unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# UI tests
#############################################################################

pushd .
cd ui/test
./test_ui
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "UI unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Enttec wing tests
#############################################################################

pushd .
cd plugins/ewinginput/test
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_ewing
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec wing unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Enttec DMXUSB Plugin test
#############################################################################

pushd .
cd plugins/enttecdmxusbout/unix/test/plugin
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src \
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbout
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Out unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Enttec DMXUSB Pro test
#############################################################################

pushd .
cd plugins/enttecdmxusbout/unix/test/pro
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src \
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbpro
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Pro unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Enttec DMXUSB Open test
#############################################################################

pushd .
cd plugins/enttecdmxusbout/unix/test/open
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src \
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbopen
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Open unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# MIDI Input tests
#############################################################################

pushd .
cd plugins/midiinput/common/test
./test_common
RESULT=$?
if [ $RESULT != 0 ]; then
    echo "MIDI Input common unit test failed ($RESULT). Please fix before commit."
    exit $RESULT
fi
popd

#############################################################################
# Final judgment
#############################################################################

echo "Unit tests passed."
