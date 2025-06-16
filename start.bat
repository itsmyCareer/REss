@echo off
rem ================================
rem run_chromedriver.bat
rem chromedriver.exe 가 있는 디렉터리에 두세요
rem 포트도 필요하면 %PORT% 값을 수정
rem ================================

set PORT=9515

rem 스크립트 위치로 현재 디렉터리 변경
cd /d "%~dp0"

:START
echo [%DATE% %TIME%] ▶ Starting ChromeDriver on port %PORT% ...
chromedriver.exe --port=%PORT%

echo [%DATE% %TIME%] ⚠️ ChromeDriver exited with code %ERRORLEVEL%.
echo Waiting 5 seconds before restart...
timeout /t 5 /nobreak >nul
goto START