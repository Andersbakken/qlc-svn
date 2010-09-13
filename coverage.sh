#!/bin/bash

#############################################################################
# Enttec DMXUSB
#############################################################################
mkdir -p libs/enttecdmxusbout/unix/coverage/html

# DMXUSB Open
lcov -d libs/enttecdmxusbout/unix/src --zerocounters
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:libs/enttecdmxusbout/unix/src libs/enttecdmxusbout/unix/test/open/test_dmxusbopen
lcov -d libs/enttecdmxusbout/unix/src --capture --output-file libs/enttecdmxusbout/unix/coverage/dmxusbopen.info

# DMXUSB Pro
lcov -d libs/enttecdmxusbout/unix/src --zerocounters
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:libs/enttecdmxusbout/unix/src libs/enttecdmxusbout/unix/test/pro/test_dmxusbpro
lcov -d libs/enttecdmxusbout/unix/src --capture --output-file libs/enttecdmxusbout/unix/coverage/dmxusbpro.info

# Generate combined HTML report
lcov -a libs/enttecdmxusbout/unix/coverage/dmxusbopen.info \
	-a libs/enttecdmxusbout/unix/coverage/dmxusbpro.info \
	-o libs/enttecdmxusbout/unix/coverage/dmxusbout.info
genhtml -o libs/enttecdmxusbout/unix/coverage/html libs/enttecdmxusbout/unix/coverage/dmxusbout.info

#############################################################################
# Enttec Wing
#############################################################################
mkdir -p libs/ewinginput/coverage/html

lcov -d libs/ewinginput/src --zerocounters
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:libs/ewinginput/src libs/ewinginput/test/test_ewing
lcov -d libs/ewinginput/src --capture --output-file libs/ewinginput/coverage/ewinginput.info

# Generate HTML report
genhtml -o libs/ewinginput/coverage/html libs/ewinginput/coverage/ewinginput.info
