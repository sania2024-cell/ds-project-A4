@echo off
REM Setup script for Property Finder project on Windows

echo ===================================
echo Property Finder - Project Setup
echo ===================================

REM Check Python installation
echo Checking Python installation...
python --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: Python not found. Please install Python 3.8 or higher.
    exit /b 1
)

for /f "tokens=*" %%i in ('python --version') do set PYTHON_VERSION=%%i
echo Python found: %PYTHON_VERSION%

REM Check CMake installation
echo Checking CMake installation...
cmake --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: CMake not found. Please install CMake 3.15 or higher.
    exit /b 1
)

for /f "tokens=*" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
echo CMake found: %CMAKE_VERSION%

REM Check C++ compiler
echo Checking C++ compiler...
g++ --version >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "tokens=*" %%i in ('g++ --version ^| head -n1') do set COMPILER_VERSION=%%i
    echo GCC found: %COMPILER_VERSION%
) else (
    cl >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo MSVC found
    ) else (
        echo Error: No C++ compiler found. Please install Visual Studio or MinGW.
        exit /b 1
    )
)

REM Create directories
echo Creating project directories...
if not exist "data" mkdir data
if not exist "docs\plots" mkdir docs\plots
if not exist "build" mkdir build

REM Setup Python environment
echo Setting up Python environment...
cd python

REM Install Python dependencies
echo Installing Python dependencies...
python -m pip install -r requirements.txt

if %ERRORLEVEL% EQU 0 (
    echo Python dependencies installed successfully.
) else (
    echo Warning: Some Python dependencies may not have installed correctly.
)

REM Generate sample data and run analysis
echo Generating sample data...
python data_cleaner.py

echo Running ML analysis...
python ml_analyzer.py

cd ..

REM Build C++ application
echo Building C++ application...
call build.bat

echo.
echo ===================================
echo Setup completed!
echo ===================================
echo.
echo Next steps:
echo 1. Open frontend\index.html in your web browser
echo 2. (Optional) Configure Mapbox token in frontend\app.js
echo 3. Run the C++ backend: cd build ^&^& .\Release\property_finder.exe
echo.
echo For development, see docs\development_guide.md