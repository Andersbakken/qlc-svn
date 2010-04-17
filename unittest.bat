@ECHO OFF

pushd .

REM Common libraries test
cd libs/common/test/%1
test_common.exe
IF NOT ERRORLEVEL 0 exit /B %ERRORLEVEL%

popd
pushd .

REM Enttec wing test
cd libs/ewinginput/test/%1
test_ewing.exe
IF NOT ERRORLEVEL 0 exit /B %ERRORLEVEL%

popd
pushd .

REM Engine test
cd engine\test\%1
test_engine.exe
IF NOT ERRORLEVEL 0 exit /B %ERRORLEVEL%

popd
pushd .

REM UI test
cd ui\test\%1
test_ui.exe
IF NOT ERRORLEVEL 0 exit /B %ERRORLEVEL%

popd
