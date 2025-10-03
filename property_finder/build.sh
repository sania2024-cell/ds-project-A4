# Build script for Property Finder C++ application

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring build with CMake..."
cmake ..

# Build the project
echo "Building the project..."
cmake --build . --config Release

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build completed successfully!"
    echo "Executable location: ./property_finder"
    echo ""
    echo "To run the application:"
    echo "  ./property_finder"
    echo ""
    echo "Make sure to:"
    echo "1. Run Python data processing first: cd ../python && python data_cleaner.py"
    echo "2. Have cleaned_properties.csv in the data/ directory"
    echo "3. Open frontend/index.html in a web browser"
else
    echo "Build failed! Please check the error messages above."
    exit 1
fi