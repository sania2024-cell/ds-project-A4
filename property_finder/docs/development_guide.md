# Property Finder Development Guide

## Project Overview

This is a comprehensive C++ Property Finder application with machine learning capabilities and web-based map integration. The project combines C++ backend processing with Python data analysis and a modern web frontend.

## Architecture

```plaintext
Frontend (HTML/CSS/JS) ←→ C++ Backend ←→ Python ML Processing
         ↑                     ↓
    Mapbox Maps            CSV Data Files
```

## Development Setup

### Prerequisites

1. **C++ Development Environment**
   - GCC/Clang compiler with C++17 support
   - CMake 3.15 or higher
   - (Optional) mlpack or Dlib for advanced ML

2. **Python Environment**
   - Python 3.8 or higher
   - Required packages (see `python/requirements.txt`)

3. **Web Development**
   - Modern web browser
   - (Optional) Local web server (Python, Node.js, or Live Server)

### Installation Steps

#### 1. Clone and Setup

```bash
cd property_finder
```

#### 2. Setup Python Environment

```bash
cd python
pip install -r requirements.txt
```

#### 3. Generate Sample Data and Clean

```bash
python data_cleaner.py
python ml_analyzer.py
```

#### 4. Build C++ Backend

```bash
mkdir build
cd build
cmake ..
make  # or cmake --build . on Windows
```

#### 5. Run the Application

```bash
# Run C++ backend
./property_finder

# Open frontend in browser
# Navigate to frontend/index.html
```

## Development Workflow

### 1. Data Processing (Python)

**File: `python/data_cleaner.py`**

- Loads and cleans raw property data
- Handles missing values and data validation
- Exports cleaned CSV for C++ backend

**File: `python/ml_analyzer.py`**

- Performs exploratory data analysis
- Trains multiple ML models
- Exports model parameters for C++ implementation

### 2. C++ Backend Development

**Core Components:**

- `Property` class: Data structure for property information
- `PropertyManager`: Manages property collections and CSV I/O
- `SearchEngine`: Advanced search and recommendation algorithms
- `MLPredictor`: Machine learning price prediction
- `APIServer`: HTTP API endpoints for frontend communication

**Key Features:**

- Fast property search with multiple filters
- ML-based price prediction
- Recommendation system based on property similarity
- RESTful API for frontend integration
- Efficient CSV data loading and processing

### 3. Frontend Development

**Technology Stack:**

- HTML5 with semantic markup
- CSS3 with modern features (Grid, Flexbox, Variables)
- Vanilla JavaScript (ES6+)
- Mapbox GL JS for interactive maps

**Features:**

- Responsive design for desktop and mobile
- Real-time property search and filtering
- Interactive map visualization
- ML price prediction interface
- Statistics dashboard

## API Endpoints

The C++ backend provides the following REST endpoints:

### Search Properties

```http
GET /search?city=Mumbai&min_price=1000000&max_price=5000000&bedrooms=3
```

### Price Prediction

```http
GET /predict?property_id=123
```

### Recommendations

```http
GET /recommend?property_id=123&max_results=5
```

### Nearby Properties

```http
GET /nearby?lat=19.0760&lon=72.8777&radius=10
```

### Statistics

```http
GET /stats
```

## File Structure

```plaintext
property_finder/
├── src/                    # C++ source files
│   ├── main.cpp           # Main application entry point
│   ├── property.*         # Property class implementation
│   ├── property_manager.* # Property collection management
│   ├── search_engine.*    # Search and recommendation logic
│   ├── ml_predictor.*     # ML prediction implementation
│   ├── api_server.*       # HTTP API server
│   ├── csv_parser.*       # CSV file handling
│   └── utils.h            # Utility functions
│
├── python/                 # Python data processing
│   ├── data_cleaner.py    # Data cleaning script
│   ├── ml_analyzer.py     # ML analysis and model training
│   └── requirements.txt   # Python dependencies
│
├── frontend/               # Web frontend
│   ├── index.html         # Main HTML page
│   ├── styles.css         # CSS styles
│   └── app.js             # JavaScript application logic
│
├── data/                   # Data files
│   ├── raw_properties.csv # Raw property data
│   ├── cleaned_properties.csv # Cleaned data for C++
│   ├── model_params.txt   # ML model parameters
│   └── encoders.txt       # Categorical encodings
│
├── docs/                   # Documentation and reports
│   ├── plots/             # Generated analysis plots
│   └── *.md               # Analysis reports
│
└── build/                  # Build output directory
```

## Configuration

### Mapbox Integration

1. Sign up for a free Mapbox account at <https://mapbox.com>
2. Get your access token
3. Update `mapboxToken` in `frontend/app.js`

### API Configuration

- Default backend runs on `http://localhost:8080`
- Update `apiBaseUrl` in `frontend/app.js` if different

## Data Format

### Input CSV Format

```csv
ID,City,Price,Bedrooms,Bathrooms,Size,Type,Latitude,Longitude,Amenities
1,Mumbai,8500000,3,2,1200,Apartment,19.0760,72.8777,"Gym,Pool,Parking"
```

### Required Columns

- **ID**: Unique property identifier
- **City**: Property city name
- **Price**: Property price in INR
- **Bedrooms**: Number of bedrooms
- **Bathrooms**: Number of bathrooms
- **Size**: Property size in square feet
- **Type**: Property type (Apartment, House, Villa, etc.)
- **Latitude/Longitude**: Geographic coordinates
- **Amenities**: Comma-separated amenities list

## Machine Learning

### Supported Models

1. **Linear Regression**: Fast, interpretable baseline
2. **Random Forest**: Better accuracy with feature importance
3. **Gradient Boosting**: Advanced ensemble method

### Features Used

- City (encoded)
- Property Type (encoded)
- Number of bedrooms/bathrooms
- Property size
- Geographic coordinates
- Derived features (size per bedroom, bathroom ratio)

### Model Export

Python models are exported to text format for C++ implementation:

- `model_params.txt`: Model coefficients and parameters
- `encoders.txt`: Categorical variable mappings

## Testing

### C++ Backend Testing

```bash
# Build and run with test data
cd build
./property_finder

# Test API endpoints
curl http://localhost:8080/search
curl http://localhost:8080/stats
```

### Frontend Testing

1. Open `frontend/index.html` in a web browser
2. Test search functionality with sample data
3. Verify map integration (requires Mapbox token)
4. Test price prediction feature

## Performance Optimization

### C++ Backend

- Use STL containers efficiently
- Implement spatial indexing for geographic queries
- Cache frequently accessed data
- Consider multithreading for large datasets

### Frontend

- Lazy loading for large property lists
- Debounced search to reduce API calls
- Map clustering for many markers
- CSS and JS minification for production

## Deployment

### Local Development

- C++ backend runs on localhost:8080
- Frontend can be served with any local web server
- Python scripts run offline for data processing

### Production Considerations

- Compile C++ with optimizations (`-O3`)
- Use proper HTTP server (nginx, Apache) for frontend
- Implement proper error handling and logging
- Add input validation and sanitization
- Use HTTPS in production

## Troubleshooting

### Common Issues

1. **Build Errors**
   - Ensure C++17 support in compiler
   - Check CMake version compatibility
   - Install required dependencies

2. **Python Dependencies**

   ```bash
   pip install --upgrade pip
   pip install -r requirements.txt
   ```

3. **Map Not Loading**
   - Check Mapbox token configuration
   - Verify internet connection
   - Check browser console for errors

4. **API Connection Issues**
   - Ensure C++ backend is running
   - Check port configuration (default 8080)
   - Verify CORS settings for cross-origin requests

### Debug Mode

Enable debug output in C++ with compiler flag:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Contributing

1. Follow C++ coding standards (Google Style Guide)
2. Add unit tests for new functionality
3. Update documentation for API changes
4. Test cross-platform compatibility
5. Maintain Python code style (PEP 8)

## License

This project is licensed under the MIT License. See LICENSE file for details.

## Resources

- [C++ Reference](https://cppreference.com/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Mapbox GL JS API](https://docs.mapbox.com/mapbox-gl-js/)
- [scikit-learn Documentation](https://scikit-learn.org/)
- [Pandas Documentation](https://pandas.pydata.org/)
