@echo off
REM Build script for Property Finder C++ application on Windows

REM Create build directory
if not exist "build" mkdir build

cd build

REM Configure with CMake
echo Configuring build with CMake...
cmake ..

REM Build the project
echo Building the project...
cmake --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% EQU 0 (
    echo Build completed successfully!
    echo Executable location: .\Release\property_finder.exe
    echo.
    echo To run the application:
    echo   .\Release\property_finder.exe
    echo.
    echo Make sure to:
    echo 1. Run Python data processing first: cd ..\python ^&^& python data_cleaner.py
    echo 2. Have cleaned_properties.csv in the data\ directory
    echo 3. Open frontend\index.html in a web browser
) else (
    echo Build failed! Please check the error messages above.
    exit /b 1
)