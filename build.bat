@echo off
echo =======================================================
echo          Building RailEase (Console and Web)
echo =======================================================
echo.

echo [*] Compiling original console application (railway.exe)...
g++ -std=c++17 railway.cpp -o railway.exe
if %ERRORLEVEL% neq 0 (
    echo [-] Failed to compile railway.cpp
    exit /b %ERRORLEVEL%
)
echo [+] railway.exe built successfully!
echo.

echo [*] Compiling web server application (server.exe)...
g++ -std=c++17 -Iinclude server.cpp -lws2_32 -o server.exe
if %ERRORLEVEL% neq 0 (
    echo [-] Failed to compile server.cpp
    exit /b %ERRORLEVEL%
)
echo [+] server.exe built successfully!
echo.
echo =======================================================
echo  Build complete!
echo  - Run "run.bat" to start the web application.
echo  - Run "railway.exe" to use the original console menu.
echo =======================================================
