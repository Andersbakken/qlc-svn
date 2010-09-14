@ECHO OFF

pushd .

REM Enttec wing test
SET OLDPATH=%PATH%
PATH=%PATH%;plugins\ewinginput\src\%1
plugins\ewinginput\test\%1\test_ewing.exe
SET PATH=%OLDPATH%
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
