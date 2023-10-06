@echo on

for /f "usebackq tokens=3" %%i in (`findstr PHP_MAJOR_VERSION main\php_version.h`) do set BRANCH=%%i
for /f "usebackq tokens=3" %%i in (`findstr PHP_MINOR_VERSION main\php_version.h`) do set BRANCH=%BRANCH%.%%i

echo %BRANCH%

if /i "%BRANCH%" equ "8.4" (
	set BRANCH=master
)

echo %BRANCH%
