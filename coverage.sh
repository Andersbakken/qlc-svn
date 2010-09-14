#!/bin/bash

#############################################################################
# Enttec DMXUSB
#############################################################################
mkdir -p plugins/enttecdmxusbout/unix/coverage/html

# DMXUSB Open
lcov -d plugins/enttecdmxusbout/unix/src --zerocounters
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src plugins/enttecdmxusbout/unix/test/open/test_dmxusbopen
lcov -d plugins/enttecdmxusbout/unix/src --capture --output-file plugins/enttecdmxusbout/unix/coverage/dmxusbopen.info

# DMXUSB Pro
lcov -d plugins/enttecdmxusbout/unix/src --zerocounters
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/enttecdmxusbout/unix/src plugins/enttecdmxusbout/unix/test/pro/test_dmxusbpro
lcov -d plugins/enttecdmxusbout/unix/src --capture --output-file plugins/enttecdmxusbout/unix/coverage/dmxusbpro.info

# Generate combined HTML report
lcov -a plugins/enttecdmxusbout/unix/coverage/dmxusbopen.info \
	-a plugins/enttecdmxusbout/unix/coverage/dmxusbpro.info \
	-o plugins/enttecdmxusbout/unix/coverage/dmxusbout.info
genhtml -o plugins/enttecdmxusbout/unix/coverage/html plugins/enttecdmxusbout/unix/coverage/dmxusbout.info

#############################################################################
# Enttec Wing
#############################################################################
mkdir -p plugins/ewinginput/coverage/html

lcov -d plugins/ewinginput/src --zerocounters
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:plugins/ewinginput/src plugins/ewinginput/test/test_ewing
lcov -d plugins/ewinginput/src --capture --output-file plugins/ewinginput/coverage/ewinginput.info

# Generate HTML report
genhtml -o plugins/ewinginput/coverage/html plugins/ewinginput/coverage/ewinginput.info
