#!/bin/bash

# Setup script for Property Finder project

echo "==================================="
echo "Property Finder - Project Setup"
echo "==================================="

# Check Python installation
echo "Checking Python installation..."
if command -v python3 &> /dev/null; then
    PYTHON_CMD=python3
elif command -v python &> /dev/null; then
    PYTHON_CMD=python
else
    echo "Error: Python not found. Please install Python 3.8 or higher."
    exit 1
fi

echo "Python found: $($PYTHON_CMD --version)"

# Check CMake installation
echo "Checking CMake installation..."
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake not found. Please install CMake 3.15 or higher."
    exit 1
fi

echo "CMake found: $(cmake --version | head -n1)"

# Check C++ compiler
echo "Checking C++ compiler..."
if command -v g++ &> /dev/null; then
    echo "GCC found: $(g++ --version | head -n1)"
elif command -v clang++ &> /dev/null; then
    echo "Clang found: $(clang++ --version | head -n1)"
else
    echo "Error: No C++ compiler found. Please install GCC or Clang."
    exit 1
fi

# Create directories
echo "Creating project directories..."
mkdir -p data
mkdir -p docs/plots
mkdir -p build

# Setup Python environment
echo "Setting up Python environment..."
cd python

# Install Python dependencies
echo "Installing Python dependencies..."
$PYTHON_CMD -m pip install -r requirements.txt

if [ $? -eq 0 ]; then
    echo "Python dependencies installed successfully."
else
    echo "Warning: Some Python dependencies may not have installed correctly."
fi

# Generate sample data and run analysis
echo "Generating sample data..."
$PYTHON_CMD data_cleaner.py

echo "Running ML analysis..."
$PYTHON_CMD ml_analyzer.py

cd ..

# Build C++ application
echo "Building C++ application..."
chmod +x build.sh
./build.sh

echo ""
echo "==================================="
echo "Setup completed!"
echo "==================================="
echo ""
echo "Next steps:"
echo "1. Open frontend/index.html in your web browser"
echo "2. (Optional) Configure Mapbox token in frontend/app.js"
echo "3. Run the C++ backend: cd build && ./property_finder"
echo ""
echo "For development, see docs/development_guide.md"