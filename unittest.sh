#!/bin/bash

pushd .

# Common library tests
cd libs/common/test
./test_common
if [ $? != 0 ]; then
	echo "Common library unit test failed ($?). Please fix before commit."
	exit
fi

popd
pushd .

# Enttec wing tests
cd libs/ewinginput/test
./test_ewing
if [ $? != 0 ]; then
	echo "Enttec wing unit test failed ($?). Please fix before commit."
	exit
fi

popd
pushd .

# Engine tests
cd engine/test
./test_engine
if [ $? != 0 ]; then
	echo "Engine unit test failed ($?). Please fix before commit."
	exit
fi

popd
pushd .

# UI tests
cd ui/test
./test_ui
if [ $? != 0 ]; then
	echo "UI unit test failed ($?). Please fix before commit."
	exit
fi

popd
echo "Unit tests passed."
