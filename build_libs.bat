@echo off
echo Cleaning and building 32-bit library...
if exist build32 (
    rmdir /s /q build32
    echo Removed build32 directory
) else (
    echo No build32 directory to remove
)
cmake -S . -B build32 -A Win32 -DCMAKE_INSTALL_PREFIX=installed32 -DENABLE_BUILDING_WRAPPERS=OFF
if %ERRORLEVEL% neq 0 (
    echo CMake configuration for 32-bit failed
    exit /b %ERRORLEVEL%
)
cmake --build build32 --config Release
if %ERRORLEVEL% neq 0 (
    echo CMake build for 32-bit failed
    exit /b %ERRORLEVEL%
)
cmake --install build32
if %ERRORLEVEL% neq 0 (
    echo CMake install for 32-bit failed
    exit /b %ERRORLEVEL%
)

echo Cleaning and building 64-bit library...
if exist build64 (
    rmdir /s /q build64
    echo Removed build64 directory
) else (
    echo No build64 directory to remove
)
cmake -S . -B build64 -A x64 -DCMAKE_INSTALL_PREFIX=installed64 -DENABLE_BUILDING_WRAPPERS=OFF
if %ERRORLEVEL% neq 0 (
    echo CMake configuration for 64-bit failed
    exit /b %ERRORLEVEL%
)
cmake --build build64 --config Release
if %ERRORLEVEL% neq 0 (
    echo CMake build for 64-bit failed
    exit /b %ERRORLEVEL%
)
cmake --install build64
if %ERRORLEVEL% neq 0 (
    echo CMake install for 64-bit failed
    exit /b %ERRORLEVEL%
)
echo Build completed successfully
exit /b 0