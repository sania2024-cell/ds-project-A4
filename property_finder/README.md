# C++ Property Finder with Machine Learning

A smart property finder application that allows users to search and filter property listings across Indian cities, predict property prices using machine learning, and visualize properties on an interactive map.

## Features

- **Smart Search**: Advanced filtering by city, price range, bedrooms, amenities
- **ML Price Prediction**: Uses machine learning to predict property prices
- **Interactive Maps**: Visualize properties on Mapbox/Google Maps
- **Data Processing**: Python-based data cleaning and ML prototyping
- **C++ Backend**: High-performance core with STL and ML libraries
- **Web Frontend**: Modern HTML/CSS/JavaScript interface

## Technology Stack

- **Backend**: C++ with mlpack/Dlib for ML
- **Data Processing**: Python (Pandas, NumPy, scikit-learn)
- **Frontend**: HTML/CSS/JavaScript with Mapbox API
- **Build System**: CMake

## Project Structure

```plaintext
property_finder/
├── data/                   # Dataset files (CSV)
├── src/                    # C++ source code
├── python/                 # Python data processing scripts
├── frontend/               # Web UI files
├── build/                  # Build output
├── docs/                   # Documentation
└── CMakeLists.txt         # Build configuration
```

## Quick Start

### Prerequisites

- C++ compiler (GCC/Clang/MSVC)
- CMake 3.15+
- Python 3.8+
- Node.js (optional, for local server)

### Setup

1. Clone and navigate to project directory
2. Install Python dependencies: `pip install -r python/requirements.txt`
3. Clean data: `python python/data_cleaner.py`
4. Build C++ backend:

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

5. Start frontend: Open `frontend/index.html` in browser

## Usage

1. Run data processing scripts to clean datasets
2. Build and run C++ backend
3. Open web interface and start searching properties
4. Use ML predictions and map visualization

## Development Workflow

1. **Data Preparation**: Use Python scripts for cleaning and EDA
2. **ML Prototyping**: Develop models in Python
3. **Backend Development**: Implement C++ core with ML integration
4. **Frontend Development**: Create web UI with map integration
5. **Integration**: Connect all components via APIs

## Contributing

Please read the development guide in `docs/` for coding standards and contribution guidelines.

## License

MIT License - see LICENSE file for details.
