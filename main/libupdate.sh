#!/bin/bash

mkdir -p qlc.app/Contents/Frameworks
cp ../libs/common/libqlccommon.3.dylib qlc.app/Contents/Frameworks
install_name_tool -change libqlccommon.3.dylib @executable_path/../Frameworks/libqlccommon.3.dylib qlc.app/Contents/MacOS/qlc
