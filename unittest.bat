@ECHO OFF

pushd .

REM Enttec wing test
SET OLDPATH=%PATH%
PATH=%PATH%;plugins\ewinginput\src
plugins\ewinginput\test\test_ewing.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
SET PATH=%OLDPATH%

popd
pushd .

REM Engine test
cd engine\test
SET OLDPATH=%PATH%
PATH=%PATH%;..\src
test_engine.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
SET PATH=%OLDPATH%

popd
pushd .

REM UI test
cd ui\test
SET OLDPATH=%PATH%
PATH=%PATH%;..\..\engine\src
test_ui.exe
IF NOT %ERRORLEVEL%==0 exit /B %ERRORLEVEL%
SET PATH=%OLDPATH%

popd
