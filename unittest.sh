#!/bin/bash

pushd .

# Common library tests
cd libs/common/test
./test_common
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Common library unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

# Enttec DMXUSB Open test
cd libs/enttecdmxusbout/unix/test/open
./test_dmxusbopen
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Open unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

# Enttec DMXUSB Pro test
cd libs/enttecdmxusbout/unix/test/pro
./test_dmxusbpro
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec DMXUSB Pro unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

# Enttec wing tests
cd libs/ewinginput/test
./test_ewing
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Enttec wing unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

# Engine tests
cd engine/test
./test_engine
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "Engine unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
pushd .

# UI tests
cd ui/test
./test_ui
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "UI unit test failed ($RESULT). Please fix before commit."
	exit $RESULT
fi

popd
echo "Unit tests passed."
