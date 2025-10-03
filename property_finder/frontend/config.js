// Property Finder Configuration
// Replace the values below with your actual API keys and settings

const CONFIG = {
    // Mapbox Configuration
    // Get your free token at: https://account.mapbox.com/access-tokens/
    MAPBOX_ACCESS_TOKEN: 'pk.eyJ1IjoiYWRpdHlhcGFyaXQiLCJhIjoiY21nNG05ZTNiMHQyYTJpcXk4aGhjMzFoeSJ9.OF-iPZnTQC8wr0J7XALfnQ',

    // Default map settings for India
    DEFAULT_MAP_CENTER: [77.1025, 28.7041], // Delhi coordinates
    DEFAULT_MAP_ZOOM: 5,

    // API Configuration
    API_BASE_URL: 'http://localhost:8080',

    // Application Settings
    PROPERTIES_PER_PAGE: 20,
    MAX_MAP_ZOOM: 15,

    // Indian Cities for Property Search
    SUPPORTED_CITIES: [
        'Mumbai', 'Delhi', 'Bangalore', 'Chennai', 'Kolkata',
        'Hyderabad', 'Pune', 'Ahmedabad', 'Jaipur', 'Surat',
        'Lucknow', 'Kanpur', 'Nagpur', 'Patna', 'Indore',
        'Thane', 'Bhopal', 'Visakhapatnam', 'Vadodara', 'Ghaziabad'
    ]
};

// Export for use in other files
if (typeof module !== 'undefined' && module.exports) {
    module.exports = CONFIG;
}