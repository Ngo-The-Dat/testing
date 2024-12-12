@echo off

:: Compiler
set CC=g++
set CSC=-std=c++17

:: Default IP and port
set ip=127.0.0.1
set port=8080

:: Build both client and server
if "%1"=="all" (
    call "%~f0" client
    call "%~f0" server
    goto :eof
)

:: Build client
if "%1"=="client" (
    echo Compiling client...
    cd client
    %CC% client.cpp -lws2_32  %CSC% -o client.exe
    if errorlevel 1 exit /b 1
    echo Client compiled successfully.
    cd ..
    goto :eof
)

:: Build server
if "%1"=="server" (
    echo Compiling server...
    cd server
    %CC% server.cpp -lws2_32 %CSC% -o server.exe
    python reset_input.py
    if errorlevel 1 exit /b 1
    echo Server compiled successfully.
    cd ..
    goto :eof
)

:: Run client
if "%1"=="run_client" (
    if not exist client\client.exe (
        echo Client executable not found. Please build it first.
        exit /b 1
    )
    echo Running client with IP=%ip% and Port=%port%...
    cd client
    client.exe %ip% %port%
    cd ..
    goto :eof
)

:: Run server
if "%1"=="run_server" (
    if not exist server\server.exe (
        echo Server executable not found. Please build it first.
        exit /b 1
    )
    echo Running server with IP=%ip% and Port=%port%...
    cd server
    server.exe %ip% %port%
    cd ..
    goto :eof
)

:: Help
if "%1"=="" (
    echo Usage: %~n0 ^<target^>
    echo Targets:
    echo   all         - Build both client and server
    echo   client      - Build the client
    echo   server      - Build the server
    echo   run_client  - Run the client (requires IP and port)
    echo   run_server  - Run the server (requires IP and port)
    exit /b 1
)

:: Invalid target
echo Unknown target: %1
exit /b 1