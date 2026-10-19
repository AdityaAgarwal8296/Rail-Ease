@echo off
echo =======================================================
echo          Starting RailEase Web Application
echo =======================================================
echo.
echo [*] Checking for server.exe...
if not exist server.exe (
    echo [-] server.exe not found! Building now...
    call build.bat
)

echo [*] Starting server on http://localhost:8080 ...
echo [*] Press Ctrl+C in this terminal to stop the server.
echo.
server.exe 8080
