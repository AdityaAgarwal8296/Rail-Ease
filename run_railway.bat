@echo off
echo =======================================================
echo          Railway Reservation System (Terminal)
echo =======================================================
echo.
echo [*] Compiling railway_terminal.cpp with g++...
g++ -std=c++11 -Wall -Wextra railway_terminal.cpp -o railway.exe
if %ERRORLEVEL% neq 0 (
    echo [-] Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)
echo [+] Build successful! Launching railway.exe...
echo.
railway.exe
echo.
echo =======================================================
echo Session ended. Press any key to close.
pause >nul
