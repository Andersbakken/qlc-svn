#!/bin/bash

#############################################################################
# Engine
#############################################################################
mkdir -p engine/coverage/html

lcov -d engine/src -c -i -o engine/coverage/enginebase.info

pushd .
cd engine/test
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_engine
popd

lcov -d engine/src -c -o engine/coverage/enginetest.info
lcov -a engine/coverage/enginebase.info \
	-a engine/coverage/enginetest.info \
	-o engine/coverage/enginemerge.info
genhtml -o engine/coverage/html engine/coverage/enginemerge.info

#############################################################################
# Enttec DMXUSB
#############################################################################
mkdir -p plugins/enttecdmxusbout/unix/coverage/html

# DMXUSB Open
lcov -d plugins/enttecdmxusbout/unix/src -c -i -o plugins/enttecdmxusbout/unix/coverage/openbase.info
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src plugins/enttecdmxusbout/unix/test/open/test_dmxusbopen
lcov -d plugins/enttecdmxusbout/unix/src -c -o plugins/enttecdmxusbout/unix/coverage/opentest.info

lcov -a plugins/enttecdmxusbout/unix/coverage/openbase.info \
        -a plugins/enttecdmxusbout/unix/coverage/opentest.info \
        -o plugins/enttecdmxusbout/unix/coverage/openmerge.info

# DMXUSB Pro
lcov -d plugins/enttecdmxusbout/unix/src -c -i -o plugins/enttecdmxusbout/unix/coverage/probase.info
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src plugins/enttecdmxusbout/unix/test/pro/test_dmxusbpro
lcov -d plugins/enttecdmxusbout/unix/src -c -o plugins/enttecdmxusbout/unix/coverage/protest.info

lcov -a plugins/enttecdmxusbout/unix/coverage/probase.info \
        -a plugins/enttecdmxusbout/unix/coverage/protest.info \
        -o plugins/enttecdmxusbout/unix/coverage/promerge.info

# Generate combined HTML report
lcov -a plugins/enttecdmxusbout/unix/coverage/openmerge.info \
	-a plugins/enttecdmxusbout/unix/coverage/promerge.info \
	-o plugins/enttecdmxusbout/unix/coverage/dmxusbout.info
genhtml -o plugins/enttecdmxusbout/unix/coverage/html plugins/enttecdmxusbout/unix/coverage/dmxusbout.info

#############################################################################
# Enttec Wing
#############################################################################
mkdir -p plugins/ewinginput/coverage/html

lcov -d plugins/ewinginput/src --zerocounters
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/ewinginput/src plugins/ewinginput/test/test_ewing
lcov -d plugins/ewinginput/src -c -o plugins/ewinginput/coverage/ewinginput.info

# Generate HTML report
genhtml -o plugins/ewinginput/coverage/html plugins/ewinginput/coverage/ewinginput.info
