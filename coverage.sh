#!/bin/bash

# Remove previous data
if [ -d coverage ]; then
	rm -rf coverage
fi

# Create directories for new coverage data
mkdir -p coverage/html

#############################################################################
# Engine
#############################################################################
lcov -d engine/src -c -i -o coverage/enginebase.info
pushd .
cd engine/test
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_engine
popd
lcov -d engine/src -c -o coverage/enginetest.info

lcov	-a coverage/enginebase.info -a coverage/enginetest.info \
	-o coverage/enginemerge.info

#############################################################################
# Enttec DMXUSB
#############################################################################
# DMXUSB Open
lcov -d plugins/enttecdmxusbout/unix/src -c -i -o coverage/dmxusbopenbase.info
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src plugins/enttecdmxusbout/unix/test/open/test_dmxusbopen
lcov -d plugins/enttecdmxusbout/unix/src -c -o coverage/dmxusbopentest.info

lcov	-a coverage/dmxusbopenbase.info -a coverage/dmxusbopentest.info \
	-o coverage/dmxusbopenmerge.info

# DMXUSB Pro
lcov -d plugins/enttecdmxusbout/unix/src -c -i -o coverage/dmxusbprobase.info
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src plugins/enttecdmxusbout/unix/test/pro/test_dmxusbpro
lcov -d plugins/enttecdmxusbout/unix/src -c -o coverage/dmxusbprotest.info

lcov	-a coverage/dmxusbprobase.info -a coverage/dmxusbprotest.info \
	-o coverage/dmxusbpromerge.info

#############################################################################
# Enttec Wing
#############################################################################
lcov -d plugins/ewinginput/src -c -i -o coverage/ewingbase.info
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/ewinginput/src plugins/ewinginput/test/test_ewing
lcov -d plugins/ewinginput/src -c -o coverage/ewingtest.info

lcov	-a coverage/ewingbase.info -a coverage/ewingtest.info \
	-o coverage/ewingmerge.info

#############################################################################
# All combined and HTMLized
#############################################################################

lcov	-a coverage/enginemerge.info \
	-a coverage/ewingmerge.info \
	-a coverage/dmxusbopenmerge.info \
	-a coverage/dmxusbpromerge.info \
	-o coverage/coverage.info

# Generate HTML report
genhtml -o coverage/html coverage/coverage.info

