#!/bin/bash

mkdir -p qlc-fixtureeditor.app/Contents/Frameworks
cp ../libs/common/libqlccommon.1.dylib qlc-fixtureeditor.app/Contents/Frameworks
install_name_tool -change libqlccommon.1.dylib @executable_path/../Frameworks/libqlccommon.1.dylib qlc-fixtureeditor.app/Contents/MacOS/qlc-fixtureeditor
