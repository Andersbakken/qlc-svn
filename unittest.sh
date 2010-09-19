#!/bin/bash

#############################################################################
# Engine tests
#############################################################################
pushd .
cd engine/test
if [ `uname -r`=="Darwin" ]; then
	DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src ./test_engine
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_engine
fi
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
if [ `uname -s` == "Darwin" ]; then
	DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src ./test_ewing
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_ewing
fi
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec wing unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Enttec DMXUSB Pro test
#############################################################################

pushd .
cd plugins/enttecdmxusbout/unix/test/pro
if [ `uname -r`=="Darwin" ]; then
	DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src ./test_dmxusbpro
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbpro
fi
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
if [ `uname -r`=="Darwin" ]; then
	DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src ./test_dmxusbopen
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbopen
fi
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Open unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Final judgment
#############################################################################

echo "Unit tests passed."
