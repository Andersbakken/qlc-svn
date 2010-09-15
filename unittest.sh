#!/bin/bash

#############################################################################
# Engine tests
#############################################################################
pushd .
cd engine/test
./test_engine
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Engine unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# UI tests
#############################################################################
ui/test/test_ui
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "UI unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

#############################################################################
# Enttec DMXUSB Pro test
#############################################################################
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src \
	plugins/enttecdmxusbout/unix/test/pro/test_dmxusbpro
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Pro unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

#############################################################################
# Enttec DMXUSB Open test
#############################################################################
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src \
	plugins/enttecdmxusbout/unix/test/open/test_dmxusbopen
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Open unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

#############################################################################
# Enttec wing tests
#############################################################################
if [ `uname -s` == "Darwin" ]; then
	DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:plugins/ewinginput/src \
		plugins/ewinginput/test/test_ewing
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/ewinginput/src \
		plugins/ewinginput/test/test_ewing
fi
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec wing unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

#############################################################################
# Final judgment
#############################################################################

echo "Unit tests passed."
