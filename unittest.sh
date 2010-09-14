#!/bin/bash

pushd .

#############################################################################
# Engine tests
#############################################################################
cd engine/test
./test_engine
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Engine unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

#############################################################################
# UI tests
#############################################################################
cd ui/test
./test_ui
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "UI unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

#############################################################################
# Enttec DMXUSB Open test
#############################################################################
cd plugins/enttecdmxusbout/unix/test/open
if [ `uname -s` != "Darwin" ]; then
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbopen
else
	DYLD_FALLBACK_LIBRARY_PATH=../../src:/Developer/Applications/Qt/plugins/imageformats \
		./test_dmxusbopen
fi
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Open unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

#############################################################################
# Enttec DMXUSB Pro test
#############################################################################
cd plugins/enttecdmxusbout/unix/test/pro
if [ `uname -s` != "Darwin" ]; then
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbpro
else
	DYLD_FALLBACK_LIBRARY_PATH=../../src:/Developer/Applications/Qt/plugins/imageformats \
		./test_dmxusbpro
fi
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Pro unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

#############################################################################
# Enttec wing tests
#############################################################################
cd plugins/ewinginput/test
if [ `uname -s` != "Darwin" ]; then
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_ewing
else
	DYLD_FALLBACK_LIBRARY_PATH=../../src:/Developer/Applications/Qt/plugins/imageformats \
		./test_ewing
fi
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec wing unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
echo "Unit tests passed."
