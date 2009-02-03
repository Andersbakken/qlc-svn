#!/bin/bash

mkdir -p qlc-fixtureeditor.app/Contents/Frameworks
cp ../libs/common/libqlccommon.3.dylib qlc-fixtureeditor.app/Contents/Frameworks
install_name_tool -change libqlccommon.3.dylib @executable_path/../Frameworks/libqlccommon.3.dylib qlc-fixtureeditor.app/Contents/MacOS/qlc-fixtureeditor
