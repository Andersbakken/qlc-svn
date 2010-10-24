#!/bin/bash

# Check if lcov is installed
if [ -z `which lcov` ]; then
    echo "Unable to produce coverage results because lcov is not installed."
fi

# Remove previous data
if [ -d coverage ]; then
    rm -rf coverage
fi

# Create directories for new coverage data
mkdir -p coverage/html

#############################################################################
# Engine
#############################################################################

# Prepare for measurement
lcov -d engine/src -c -i -o coverage/enginebase.info
if [ $? != 0 ]; then
    echo
    echo "Error running lcov. Did you run \"qmake CONFIG+=coverage\" " \
         "before compiling the sources? If not, go to the top level " \
         "source directory and run \"make distclean\", then " \
         "\"qmake CONFIG+=coverage\", and finally \"make\"." \
         "After that you can run this script to produce unit test " \
         "coverage results."
    exit
fi

# Run the unit test
pushd .
cd engine/test
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_engine
popd

# Measure coverage and combine results from before and after the unit test
lcov -d engine/src -c -o coverage/enginetest.info
lcov -a coverage/enginebase.info -a coverage/enginetest.info \
     -o coverage/enginemerge.info

#############################################################################
# Enttec DMXUSB
#############################################################################

##############
# The plugin #
##############

# Prepare for measurement
lcov -d plugins/enttecdmxusbout/unix/src -c -i -o coverage/dmxusboutbase.info

# Run the unit test
pushd .
cd plugins/enttecdmxusbout/unix/test/plugin
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbout
popd

# Measure coverage and combine results from before and after the unit test
lcov -d plugins/enttecdmxusbout/unix/src -c -o coverage/dmxusbouttest.info
lcov -a coverage/dmxusboutbase.info -a coverage/dmxusbouttest.info \
     -o coverage/dmxusboutmerge.info

###############
# DMXUSB Open #
###############

# Prepare for measurement
lcov -d plugins/enttecdmxusbout/unix/src -c -i -o coverage/dmxusbopenbase.info

# Run the unit test
pushd .
cd plugins/enttecdmxusbout/unix/test/open
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbopen
popd

# Measure coverage and combine results from before and after the unit test
lcov -d plugins/enttecdmxusbout/unix/src -c -o coverage/dmxusbopentest.info
lcov -a coverage/dmxusbopenbase.info -a coverage/dmxusbopentest.info \
     -o coverage/dmxusbopenmerge.info

##############
# DMXUSB Pro #
##############

# Prepare for measurement
lcov -d plugins/enttecdmxusbout/unix/src -c -i -o coverage/dmxusbprobase.info

# Run the unit test
pushd .
cd plugins/enttecdmxusbout/unix/test/pro
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../src \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../src ./test_dmxusbpro
popd

# Measure coverage and combine results from before and after the unit test
lcov -d plugins/enttecdmxusbout/unix/src -c -o coverage/dmxusbprotest.info
lcov -a coverage/dmxusbprobase.info -a coverage/dmxusbprotest.info \
     -o coverage/dmxusbpromerge.info

#############################################################################
# Enttec Wing
#############################################################################

# Prepare for measurement
lcov -d plugins/ewinginput/src -c -i -o coverage/ewingbase.info

# Run the unit test
pushd .
cd plugins/ewinginput/test
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_ewing
popd

# Measure coverage and combine results from before and after the unit test
lcov -d plugins/ewinginput/src -c -o coverage/ewingtest.info
lcov -a coverage/ewingbase.info -a coverage/ewingtest.info \
     -o coverage/ewingmerge.info

#############################################################################
# MIDI Input
#############################################################################

##########
# Common #
##########

# Prepare for measurement
lcov -d plugins/midiinput/common/src -c -i -o coverage/midiinputcommonbase.info

# Run the unit test
pushd .
cd plugins/midiinput/common/test
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./test_common
popd

# Measure coverage and combine results from before and after the unit test
lcov -d plugins/midiinput/common/src -c -o coverage/midiinputcommontest.info
lcov -a coverage/midiinputcommonbase.info -a coverage/midiinputcommontest.info \
     -o coverage/midiinputcommonmerge.info

#############################################################################
# All combined and HTMLized
#############################################################################

lcov -a coverage/enginemerge.info \
     -a coverage/ewingmerge.info \
     -a coverage/dmxusbopenmerge.info \
     -a coverage/dmxusbpromerge.info \
     -a coverage/midiinputcommonmerge.info \
     -o coverage/coverage.info

# Remove stuff that isn't part of QLC sources
lcov -r coverage/coverage.info moc_* -o coverage/coverage.info
lcov -r coverage/coverage.info *usr* -o coverage/coverage.info
lcov -r coverage/coverage.info *Library* -o coverage/coverage.info # OSX

# Generate HTML report
genhtml -o coverage/html coverage/coverage.info
