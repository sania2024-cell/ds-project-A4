// Property Finder - Main JavaScript Application

class PropertyFinder {
    constructor() {
        // Load configuration
        this.config = typeof CONFIG !== 'undefined' ? CONFIG : {
            MAPBOX_ACCESS_TOKEN: 'pk.eyJ1IjoiYWRpdHlhcGFyaXQiLCJhIjoiY21nNG05ZTNiMHQyYTJpcXk4aGhjMzFoeSJ9.OF-iPZnTQC8wr0J7XALfnQ',
            DEFAULT_MAP_CENTER: [77.1025, 28.7041],
            DEFAULT_MAP_ZOOM: 5,
            API_BASE_URL: 'http://localhost:8080',
            PROPERTIES_PER_PAGE: 1000,
            USE_SAMPLE_DATA: true
        };

        this.apiBaseUrl = this.config.API_BASE_URL;
        this.allProperties = []; // Store all properties for filtering
        this.currentProperties = []; // Store filtered/displayed properties
        this.currentPage = 0;
        this.pageSize = this.config.PROPERTIES_PER_PAGE;
        this.map = null;
        this.markers = [];
        this.mapboxToken = this.config.MAPBOX_ACCESS_TOKEN;

        this.init();
    }

    init() {
        this.setupEventListeners();
        this.loadInitialData();
        this.setupNavigation();
        this.initializeMap();
    }

    setupEventListeners() {
        // Search form
        document.getElementById('search-btn').addEventListener('click', () => this.searchProperties());
        document.getElementById('clear-btn').addEventListener('click', () => this.clearSearch());

        // Sort functionality
        document.getElementById('sort-by').addEventListener('change', (e) => this.sortProperties(e.target.value));

        // Prediction form
        document.getElementById('predict-btn').addEventListener('click', () => this.predictPrice());

        // Map controls
        document.getElementById('show-all-properties').addEventListener('click', () => this.showAllPropertiesOnMap());
        document.getElementById('find-nearby').addEventListener('click', () => this.findNearbyProperties());

        // Load more
        document.getElementById('load-more-btn').addEventListener('click', () => this.loadMoreProperties());

        // Enter key support
        document.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                const activeSection = document.querySelector('.section.active').id;
                if (activeSection === 'search') {
                    this.searchProperties();
                } else if (activeSection === 'predict') {
                    this.predictPrice();
                }
            }
        });
    }

    setupNavigation() {
        const navLinks = document.querySelectorAll('.nav-link');
        const sections = document.querySelectorAll('.section');

        navLinks.forEach(link => {
            link.addEventListener('click', (e) => {
                e.preventDefault();

                // Remove active class from all links and sections
                navLinks.forEach(l => l.classList.remove('active'));
                sections.forEach(s => s.classList.remove('active'));

                // Add active class to clicked link
                link.classList.add('active');

                // Show corresponding section
                const targetId = link.getAttribute('href').substring(1);
                const targetSection = document.getElementById(targetId);
                if (targetSection) {
                    targetSection.classList.add('active');

                    // Initialize section-specific functionality
                    if (targetId === 'map' && this.map) {
                        setTimeout(() => this.map.resize(), 100);
                    } else if (targetId === 'stats') {
                        this.loadStatistics();
                    }
                }
            });
        });
    }

    initializeMap() {
        // Check if Mapbox token is configured
        if (!this.mapboxToken || this.mapboxToken === 'YOUR_MAPBOX_ACCESS_TOKEN_HERE') {
            this.showMapError('Please configure your Mapbox access token in config.js');
            return;
        }

        // Initialize Mapbox map
        try {
            mapboxgl.accessToken = this.mapboxToken;

            this.map = new mapboxgl.Map({
                container: 'map-view',
                style: 'mapbox://styles/mapbox/streets-v12', // Updated to latest streets style
                center: this.config.DEFAULT_MAP_CENTER, // Delhi coordinates (center of India)
                zoom: this.config.DEFAULT_MAP_ZOOM, // Show more of India
                pitch: 0,
                bearing: 0
            });

            // Add map controls
            this.map.addControl(new mapboxgl.NavigationControl(), 'top-right');
            this.map.addControl(new mapboxgl.FullscreenControl(), 'top-right');
            this.map.addControl(new mapboxgl.ScaleControl(), 'bottom-left');
            this.map.addControl(new mapboxgl.GeolocateControl({
                positionOptions: {
                    enableHighAccuracy: true
                },
                trackUserLocation: true,
                showUserHeading: true
            }), 'top-right');

            // Add map event handlers
            this.map.on('load', () => {
                console.log('Map loaded successfully');
                this.showToast('Map loaded successfully!', 'success');
                this.loadPropertiesOnMap();
            });

            this.map.on('click', (e) => {
                console.log('Map clicked at:', e.lngLat.lng, e.lngLat.lat);
                // You can add property search at clicked location here
            });

            this.map.on('error', (e) => {
                console.error('Mapbox error:', e.error);
                this.showToast('Map error occurred', 'error');
            });

        } catch (error) {
            console.error('Mapbox initialization failed:', error);
            this.showMapError('Map initialization failed: ' + error.message);
        }
    }

    showMapError(message) {
        console.error('Map Error:', message);
        this.showToast(message, 'error');

        // Show helpful error message in map container
        const mapView = document.getElementById('map-view');
        mapView.innerHTML = `
            <div style="display: flex; align-items: center; justify-content: center; height: 100%; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; text-align: center; padding: 2rem;">
                <div>
                    <i class="fas fa-map-marker-alt" style="font-size: 4rem; margin-bottom: 1.5rem; opacity: 0.8;"></i>
                    <h3 style="margin-bottom: 1rem; font-weight: 300;">Map Configuration Required</h3>
                    <p style="margin-bottom: 1.5rem; opacity: 0.9;">${message}</p>
                    <div style="background: rgba(255,255,255,0.1); padding: 1.5rem; border-radius: 8px; margin: 1rem 0;">
                        <p style="font-weight: bold; margin-bottom: 0.5rem;">To enable maps:</p>
                        <ol style="text-align: left; margin: 0; padding-left: 1.5rem;">
                            <li style="margin-bottom: 0.5rem;">Visit <a href="https://account.mapbox.com/" target="_blank" style="color: #fff; text-decoration: underline;">account.mapbox.com</a></li>
                            <li style="margin-bottom: 0.5rem;">Create a free account or sign in</li>
                            <li style="margin-bottom: 0.5rem;">Get your access token</li>
                            <li>Replace 'YOUR_MAPBOX_ACCESS_TOKEN_HERE' in config.js</li>
                        </ol>
                    </div>
                    <button onclick="window.open('https://account.mapbox.com/', '_blank')" 
                            style="background: #fff; color: #667eea; border: none; padding: 0.75rem 1.5rem; border-radius: 4px; cursor: pointer; font-weight: bold;">Get Mapbox Token</button>
                </div>
                </div>
            </div>
        `;
    }

    loadPropertiesOnMap() {
        // Load and display properties on the map
        if (!this.map || !this.currentProperties.length) {
            return;
        }

        // Clear existing markers
        this.clearMapMarkers();

        // Add markers for current properties
        this.currentProperties.forEach(property => {
            if (property.latitude && property.longitude) {
                this.addPropertyMarker(property);
            }
        });

        // Fit map to show all properties
        if (this.currentProperties.length > 0) {
            this.fitMapToProperties();
        }
    }

    addPropertyMarker(property) {
        if (!this.map) return;

        // Create custom marker HTML
        const markerElement = document.createElement('div');
        markerElement.className = 'property-marker';
        markerElement.innerHTML = `
            <div class="marker-content">
                <div class="price">₹${this.formatPrice(property.price)}</div>
                <div class="type">${property.type}</div>
            </div>
        `;

        // Create popup with property details
        const popup = new mapboxgl.Popup({ offset: 25 })
            .setHTML(`
                <div class="map-popup">
                    <h3>${property.title || property.type + ' in ' + property.city}</h3>
                    <p><strong>Price:</strong> ₹${this.formatPrice(property.price)}</p>
                    <p><strong>Location:</strong> ${property.area}, ${property.city}</p>
                    <p><strong>Size:</strong> ${property.size} sq ft</p>
                    <p><strong>Bedrooms:</strong> ${property.bedrooms}</p>
                    <button onclick="app.viewPropertyDetails(${property.id})" class="btn btn-primary btn-sm">View Details</button>
                </div>
            `);

        // Create and add marker
        const marker = new mapboxgl.Marker(markerElement)
            .setLngLat([property.longitude, property.latitude])
            .setPopup(popup)
            .addTo(this.map);

        this.markers.push(marker);
    }

    clearMapMarkers() {
        this.markers.forEach(marker => marker.remove());
        this.markers = [];
    }

    fitMapToProperties() {
        if (!this.map || !this.currentProperties.length) return;

        const coordinates = this.currentProperties
            .filter(p => p.latitude && p.longitude)
            .map(p => [p.longitude, p.latitude]);

        if (coordinates.length === 0) return;

        if (coordinates.length === 1) {
            this.map.flyTo({
                center: coordinates[0],
                zoom: 12
            });
        } else {
            const bounds = new mapboxgl.LngLatBounds();
            coordinates.forEach(coord => bounds.extend(coord));

            this.map.fitBounds(bounds, {
                padding: { top: 50, bottom: 50, left: 50, right: 50 },
                maxZoom: 15
            });
        }
    } async loadInitialData() {
        try {
            this.showLoading(true);

            if (!this.config.USE_SAMPLE_DATA) {
                try {
                    const response = await this.fetchWithTimeout(`${this.apiBaseUrl}/search`, 3000);
                    if (response.ok) {
                        const data = await response.json();
                        this.allProperties = data.properties || [];
                        this.currentProperties = [...this.allProperties];
                        this.displayProperties(this.currentProperties);
                        this.showToast(`Loaded ${this.currentProperties.length} properties`, 'success');
                        return;
                    }
                } catch (apiError) {
                    console.log('API not available, loading from CSV');
                }
            }

            // Load from CSV file
            await this.loadSampleData();
        } catch (error) {
            console.error('Failed to load data:', error);
            this.showToast('Failed to load properties', 'error');
        } finally {
            this.showLoading(false);
        }
    }

    async loadSampleData() {
        console.log('Loading sample properties...');
        try {
            const response = await fetch('../data/sample_properties.csv');
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const csvText = await response.text();
            console.log('CSV file loaded successfully');
            const lines = csvText.split('\n');
            console.log(`Found ${lines.length} lines in CSV`);
            const headers = lines[0].split(',');
            console.log('CSV Headers:', headers);
            
            this.allProperties = lines.slice(1)
                .filter(line => line.trim()) // Skip empty lines
                .map(line => {
                    const values = line.split(',');
                    const property = {};
                    
                    headers.forEach((header, index) => {
                        const value = values[index];
                        if (header === 'ID') {
                            property.id = parseInt(value);
                        } else if (header === 'Price') {
                            property.price = parseInt(value);
                        } else if (header === 'Bedrooms' || header === 'Bathrooms') {
                            property[header.toLowerCase()] = parseInt(value);
                        } else if (header === 'Size') {
                            property.size = parseInt(value);
                        } else if (header === 'Latitude' || header === 'Longitude') {
                            property[header.toLowerCase()] = parseFloat(value);
                        } else if (header === 'Amenities') {
                            property.amenities = value.replace(/"/g, '').split(',').map(a => a.trim());
                        } else {
                            property[header.toLowerCase()] = value;
                        }
                    });
                    
                    // Add predicted price
                    property.predicted_price = Math.round(property.price * 1.05);
                    return property;
                });
            
            this.currentProperties = [...this.allProperties];
            console.log(`Processed ${this.allProperties.length} properties`);
            console.log('First few properties:', this.allProperties.slice(0, 3));
                
            this.displayProperties(this.currentProperties);
            this.showToast(`Loaded ${this.currentProperties.length} properties`, 'success');
        } catch (error) {
            console.error('Failed to load CSV:', error);
            console.log('Current directory context:', window.location.href);
            // Fallback to a minimal set of properties if CSV fails to load
            this.allProperties = [{
                id: 1,
                city: "Mumbai",
                price: 8500000,
                bedrooms: 3,
                bathrooms: 2,
                size: 1200,
                type: "Apartment",
                latitude: 19.0760,
                longitude: 72.8777,
                amenities: ["Gym", "Pool", "Parking"],
                predicted_price: 8700000
            }];
            this.currentProperties = [...this.allProperties];
            this.displayProperties(this.currentProperties);
            this.showToast('Failed to load properties data', 'error');
        }
    }

    async searchProperties() {
        try {
            this.showLoading(true);

            const filters = this.getSearchFilters();
            const queryString = new URLSearchParams(filters).toString();

            console.log('Searching with filters:', filters);

            if (!this.config.USE_SAMPLE_DATA) {
                try {
                    const response = await this.fetchWithTimeout(`${this.apiBaseUrl}/search?${queryString}`, 3000);
                    if (response.ok) {
                        const data = await response.json();
                        this.currentProperties = data.properties || [];
                    } else {
                        throw new Error('API search failed');
                    }
                } catch (apiError) {
                    console.log('API search failed, filtering locally');
                    // Fallback to client-side filtering from all properties
                    this.currentProperties = this.filterPropertiesByFilters(this.allProperties, filters);
                }
            } else {
                // Client-side filtering from all properties
                this.currentProperties = this.filterPropertiesByFilters(this.allProperties, filters);
            }

            this.currentPage = 0;
            this.displayProperties(this.currentProperties);
            this.updateMapMarkers();

            const count = this.currentProperties.length;
            this.showToast(`Found ${count} matching properties`, count > 0 ? 'success' : 'warning');

        } catch (error) {
            console.error('Search error:', error);
            this.showToast('Search failed. Please try again.', 'error');
        } finally {
            this.showLoading(false);
        }
    }

    getSearchFilters() {
        const filters = {};

        const city = document.getElementById('city').value;
        const type = document.getElementById('property-type').value;
        const bedrooms = document.getElementById('bedrooms').value;
        const minPrice = document.getElementById('min-price').value;
        const maxPrice = document.getElementById('max-price').value;
        const minSize = document.getElementById('min-size').value;

        if (city) filters.city = city;
        if (type) filters.type = type;
        if (bedrooms) filters.bedrooms = bedrooms;
        if (minPrice) filters.min_price = minPrice;
        if (maxPrice) filters.max_price = maxPrice;
        if (minSize) filters.min_size = minSize;

        return filters;
    }

    filterPropertiesByFilters(properties, filters) {
        return properties.filter(property => {
            if (filters.city && property.city !== filters.city) return false;
            if (filters.type && property.type !== filters.type) return false;
            if (filters.bedrooms && property.bedrooms != filters.bedrooms) return false;
            if (filters.min_price && property.price < parseFloat(filters.min_price)) return false;
            if (filters.max_price && property.price > parseFloat(filters.max_price)) return false;
            if (filters.min_size && property.size < parseFloat(filters.min_size)) return false;
            return true;
        });
    }

    clearSearch() {
        // Clear all form fields
        document.getElementById('city').value = '';
        document.getElementById('property-type').value = '';
        document.getElementById('bedrooms').value = '';
        document.getElementById('min-price').value = '';
        document.getElementById('max-price').value = '';
        document.getElementById('min-size').value = '';

        // Reset to all properties
        this.currentProperties = [...this.allProperties];
        this.displayProperties(this.currentProperties);
        this.showToast('Search cleared - showing all properties', 'success');
    }

    sortProperties(sortBy) {
        if (!this.currentProperties.length) return;

        const sorted = [...this.currentProperties];

        switch (sortBy) {
            case 'price-asc':
                sorted.sort((a, b) => a.price - b.price);
                break;
            case 'price-desc':
                sorted.sort((a, b) => b.price - a.price);
                break;
            case 'size-asc':
                sorted.sort((a, b) => a.size - b.size);
                break;
            case 'size-desc':
                sorted.sort((a, b) => b.size - a.size);
                break;
        }

        this.currentProperties = sorted;
        this.displayProperties(this.currentProperties);
    }

    displayProperties(properties) {
        const container = document.getElementById('properties-list');
        const countElement = document.getElementById('results-count');

        countElement.textContent = `${properties.length} properties found`;

        if (properties.length === 0) {
            container.innerHTML = `
                <div style="padding: 3rem; text-align: center; color: #666;">
                    <i class="fas fa-search" style="font-size: 3rem; margin-bottom: 1rem;"></i>
                    <h3>No properties found</h3>
                    <p>Try adjusting your search criteria</p>
                </div>
            `;
            return;
        }

        // Display all properties at once
        container.innerHTML = properties.map(property => this.createPropertyCard(property)).join('');

        // Hide load more button since we're showing all properties
        const loadMoreBtn = document.getElementById('load-more-btn');
        loadMoreBtn.style.display = 'none';
    }

    createPropertyCard(property) {
        const formatPrice = (price) => {
            return new Intl.NumberFormat('en-IN', {
                style: 'currency',
                currency: 'INR',
                maximumFractionDigits: 0
            }).format(price);
        };

        const predictedPriceHtml = property.predicted_price ?
            `<div class="predicted-price">
                <i class="fas fa-robot"></i> ML Prediction: ${formatPrice(property.predicted_price)}
            </div>` : '';

        const amenitiesHtml = property.amenities && property.amenities.length > 0 ?
            `<div class="property-amenities">
                <strong>Amenities:</strong> ${property.amenities.join(', ')}
            </div>` : '';

        return `
            <div class="property-card" data-id="${property.id}">
                <div class="property-image">
                    <i class="fas fa-home"></i>
                </div>
                <div class="property-details">
                    <h4>${property.type} in ${property.city}</h4>
                    <div class="property-meta">
                        <span><i class="fas fa-bed"></i> ${property.bedrooms} BHK</span>
                        <span><i class="fas fa-bath"></i> ${property.bathrooms} Bath</span>
                        <span><i class="fas fa-ruler-combined"></i> ${property.size} sq ft</span>
                    </div>
                    <div class="property-description">
                        ${amenitiesHtml}
                        <p>Located in ${property.city}, this ${property.type.toLowerCase()} offers comfortable living space.</p>
                    </div>
                </div>
                <div class="property-price">
                    <div class="price">${formatPrice(property.price)}</div>
                    ${predictedPriceHtml}
                    <div class="property-actions">
                        <button class="btn btn-primary btn-small" onclick="app.showOnMap(${property.id})">
                            <i class="fas fa-map-marker-alt"></i> Show on Map
                        </button>
                        <button class="btn btn-secondary btn-small" onclick="app.getRecommendations(${property.id})">
                            <i class="fas fa-thumbs-up"></i> Similar
                        </button>
                    </div>
                </div>
            </div>
        `;
    }

    loadMoreProperties() {
        this.currentPage++;
        this.displayProperties(this.currentProperties);
    }

    async predictPrice() {
        try {
            const city = document.getElementById('predict-city').value;
            const type = document.getElementById('predict-type').value;
            const bedrooms = document.getElementById('predict-bedrooms').value;
            const bathrooms = document.getElementById('predict-bathrooms').value;
            const size = document.getElementById('predict-size').value;

            if (!city || !type || !bedrooms || !bathrooms || !size) {
                this.showToast('Please fill in all fields', 'warning');
                return;
            }

            this.showLoading(true);

            // Simple ML prediction based on sample data
            let predictedPrice = this.calculatePredictedPrice(city, type, parseInt(bedrooms), parseInt(bathrooms), parseFloat(size));

            try {
                // Try API prediction if available
                const response = await this.fetchWithTimeout(`${this.apiBaseUrl}/predict?property_id=1`, 3000);
                if (response.ok) {
                    const data = await response.json();
                    if (data.predicted_price) {
                        predictedPrice = data.predicted_price;
                    }
                }
            } catch (apiError) {
                // Use fallback calculation
                console.log('Using fallback price prediction');
            }

            this.displayPrediction(predictedPrice);
            this.showToast('Price predicted successfully', 'success');

        } catch (error) {
            console.error('Prediction error:', error);
            this.showToast('Prediction failed. Please try again.', 'error');
        } finally {
            this.showLoading(false);
        }
    }

    calculatePredictedPrice(city, type, bedrooms, bathrooms, size) {
        // Simple rule-based prediction for demonstration
        const cityMultipliers = {
            'Mumbai': 1.5,
            'Delhi': 1.3,
            'Bengaluru': 1.2,
            'Chennai': 1.0,
            'Kolkata': 0.8,
            'Hyderabad': 1.1,
            'Pune': 1.0,
            'Ahmedabad': 0.9
        };

        const typeMultipliers = {
            'Apartment': 1.0,
            'House': 1.2,
            'Villa': 1.5,
            'Studio': 0.8
        };

        const basePricePerSqFt = 5000; // Base price per square foot
        const cityMultiplier = cityMultipliers[city] || 1.0;
        const typeMultiplier = typeMultipliers[type] || 1.0;
        const bedroomMultiplier = 1 + (bedrooms - 1) * 0.1;
        const bathroomMultiplier = 1 + (bathrooms - 1) * 0.05;

        const predictedPrice = size * basePricePerSqFt * cityMultiplier * typeMultiplier * bedroomMultiplier * bathroomMultiplier;

        // Add some random variation (±10%)
        const variation = (Math.random() - 0.5) * 0.2;
        return Math.round(predictedPrice * (1 + variation));
    }

    displayPrediction(predictedPrice) {
        const formatPrice = (price) => {
            return new Intl.NumberFormat('en-IN', {
                style: 'currency',
                currency: 'INR',
                maximumFractionDigits: 0
            }).format(price);
        };

        document.getElementById('predicted-price').textContent = formatPrice(predictedPrice);

        const lowerRange = Math.round(predictedPrice * 0.9);
        const upperRange = Math.round(predictedPrice * 1.1);
        document.getElementById('price-range').textContent = `${formatPrice(lowerRange)} - ${formatPrice(upperRange)}`;

        const confidence = Math.round(75 + Math.random() * 20); // 75-95% confidence
        document.getElementById('confidence').textContent = `${confidence}%`;
    }

    showOnMap(propertyId) {
        const property = this.currentProperties.find(p => p.id === propertyId);
        if (!property) return;

        // Switch to map view
        document.querySelector('.nav-link[href="#map"]').click();

        // Center map on property
        if (this.map) {
            this.map.flyTo({
                center: [property.longitude, property.latitude],
                zoom: 15
            });

            // Add marker if not exists
            this.updateMapMarkers([property]);

            // Show popup
            setTimeout(() => {
                const popup = new mapboxgl.Popup()
                    .setLngLat([property.longitude, property.latitude])
                    .setHTML(`
                        <div>
                            <h4>${property.type} in ${property.city}</h4>
                            <p><strong>Price:</strong> ₹${property.price.toLocaleString()}</p>
                            <p><strong>Size:</strong> ${property.size} sq ft</p>
                            <p><strong>Bedrooms:</strong> ${property.bedrooms}</p>
                        </div>
                    `)
                    .addTo(this.map);
            }, 1000);
        }

        this.showToast(`Showing ${property.type} in ${property.city} on map`, 'success');
    }

    async getRecommendations(propertyId) {
        try {
            this.showLoading(true);

            let recommendations = [];

            try {
                const response = await this.fetchWithTimeout(`${this.apiBaseUrl}/recommend?property_id=${propertyId}`, 3000);
                if (response.ok) {
                    const data = await response.json();
                    recommendations = data.properties || [];
                }
            } catch (apiError) {
                // Fallback to simple similarity matching
                const targetProperty = this.currentProperties.find(p => p.id === propertyId);
                if (targetProperty) {
                    recommendations = this.getSimilarProperties(targetProperty);
                }
            }

            if (recommendations.length > 0) {
                this.currentProperties = recommendations;
                this.displayProperties(recommendations);
                this.showToast(`Found ${recommendations.length} similar properties`, 'success');
            } else {
                this.showToast('No similar properties found', 'warning');
            }

        } catch (error) {
            console.error('Recommendations error:', error);
            this.showToast('Failed to get recommendations', 'error');
        } finally {
            this.showLoading(false);
        }
    }

    getSimilarProperties(targetProperty) {
        return this.currentProperties
            .filter(p => p.id !== targetProperty.id)
            .filter(p =>
                p.city === targetProperty.city ||
                p.type === targetProperty.type ||
                Math.abs(p.bedrooms - targetProperty.bedrooms) <= 1
            )
            .slice(0, 5);
    }

    showAllPropertiesOnMap() {
        if (!this.map) return;

        this.updateMapMarkers(this.currentProperties);

        if (this.currentProperties.length > 0) {
            // Fit map to show all properties
            const bounds = new mapboxgl.LngLatBounds();
            this.currentProperties.forEach(property => {
                bounds.extend([property.longitude, property.latitude]);
            });
            this.map.fitBounds(bounds, { padding: 50 });
        }

        this.showToast(`Showing ${this.currentProperties.length} properties on map`, 'success');
    }

    findNearbyProperties() {
        if (!navigator.geolocation) {
            this.showToast('Geolocation not supported by browser', 'error');
            return;
        }

        this.showLoading(true);

        navigator.geolocation.getCurrentPosition(
            async (position) => {
                const { latitude, longitude } = position.coords;

                try {
                    let nearbyProperties = [];

                    try {
                        const response = await this.fetchWithTimeout(
                            `${this.apiBaseUrl}/nearby?lat=${latitude}&lon=${longitude}&radius=10`,
                            3000
                        );
                        if (response.ok) {
                            const data = await response.json();
                            nearbyProperties = data.properties || [];
                        }
                    } catch (apiError) {
                        // Fallback to distance calculation
                        nearbyProperties = this.calculateNearbyProperties(latitude, longitude, 10);
                    }

                    this.currentProperties = nearbyProperties;
                    this.displayProperties(nearbyProperties);
                    this.updateMapMarkers(nearbyProperties);

                    if (this.map) {
                        this.map.flyTo({
                            center: [longitude, latitude],
                            zoom: 12
                        });
                    }

                    this.showToast(`Found ${nearbyProperties.length} nearby properties`, 'success');

                } catch (error) {
                    console.error('Nearby search error:', error);
                    this.showToast('Failed to find nearby properties', 'error');
                } finally {
                    this.showLoading(false);
                }
            },
            (error) => {
                this.showLoading(false);
                this.showToast('Failed to get your location', 'error');
            }
        );
    }

    calculateNearbyProperties(lat, lon, radiusKm) {
        return this.currentProperties.filter(property => {
            const distance = this.calculateDistance(lat, lon, property.latitude, property.longitude);
            return distance <= radiusKm;
        });
    }

    calculateDistance(lat1, lon1, lat2, lon2) {
        const R = 6371; // Earth's radius in kilometers
        const dLat = (lat2 - lat1) * Math.PI / 180;
        const dLon = (lon2 - lon1) * Math.PI / 180;
        const a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
            Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) *
            Math.sin(dLon / 2) * Math.sin(dLon / 2);
        const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
        return R * c;
    }

    updateMapMarkers(properties = this.currentProperties) {
        if (!this.map) return;

        // Clear existing markers
        this.markers.forEach(marker => marker.remove());
        this.markers = [];

        // Add new markers
        properties.forEach(property => {
            const el = document.createElement('div');
            el.className = 'marker';
            el.style.backgroundColor = '#3498db';
            el.style.width = '20px';
            el.style.height = '20px';
            el.style.borderRadius = '50%';
            el.style.border = '2px solid white';
            el.style.boxShadow = '0 2px 5px rgba(0,0,0,0.3)';

            const marker = new mapboxgl.Marker(el)
                .setLngLat([property.longitude, property.latitude])
                .setPopup(new mapboxgl.Popup().setHTML(`
                    <div>
                        <h4>${property.type} in ${property.city}</h4>
                        <p><strong>Price:</strong> ₹${property.price.toLocaleString()}</p>
                        <p><strong>Size:</strong> ${property.size} sq ft</p>
                        <p><strong>Bedrooms:</strong> ${property.bedrooms}</p>
                    </div>
                `))
                .addTo(this.map);

            this.markers.push(marker);
        });
    }

    async loadStatistics() {
        try {
            // Ensure we have properties data
            if (this.currentProperties.length === 0) {
                await this.loadInitialData();
            }

            let stats = null;

            try {
                const response = await this.fetchWithTimeout(`${this.apiBaseUrl}/stats`, 3000);
                if (response.ok) {
                    stats = await response.json();
                }
            } catch (apiError) {
                console.log('API stats not available, calculating locally');
                // Calculate statistics from current properties
                stats = this.calculateLocalStatistics();
            }

            if (stats) {
                this.displayStatistics(stats);
            } else {
                throw new Error('No statistics available');
            }

        } catch (error) {
            console.error('Statistics error:', error);
            // Still try to display local statistics if we have properties
            if (this.currentProperties.length > 0) {
                const localStats = this.calculateLocalStatistics();
                this.displayStatistics(localStats);
            } else {
                this.showToast('Failed to load statistics', 'error');
            }
        }
    }

    formatPrice(price) {
        return new Intl.NumberFormat('en-IN', {
            maximumFractionDigits: 0
        }).format(price);
    }

    updateCharts() {
        // Check if we have properties data
        if (!this.currentProperties || this.currentProperties.length === 0) {
            console.error('No properties data available for charts');
            return;
        }

        // Ensure Chart.js and ChartManager are available
        if (typeof Chart === 'undefined' || typeof ChartManager === 'undefined') {
            console.error('Chart.js or ChartManager not loaded');
            return;
        }

        try {
            // Render the charts directly
            ChartManager.renderCityChart(this.currentProperties);
            ChartManager.renderTypeChart(this.currentProperties);
        } catch (error) {
            console.error('Error updating charts:', error);
            this.showToast('Failed to update charts', 'error');
        }
    }

    calculateLocalStatistics() {
        if (this.currentProperties.length === 0) return null;

        const totalProperties = this.currentProperties.length;
        const totalPrice = this.currentProperties.reduce((sum, p) => sum + p.price, 0);
        const avgPrice = totalPrice / totalProperties;

        const cities = [...new Set(this.currentProperties.map(p => p.city))];
        const totalCities = cities.length;

        return {
            total_properties: totalProperties,
            price_statistics: {
                mean: avgPrice,
                min: Math.min(...this.currentProperties.map(p => p.price)),
                max: Math.max(...this.currentProperties.map(p => p.price))
            },
            total_cities: totalCities
        };
    }

    displayStatistics(stats) {
        // Update statistics cards
        document.getElementById('total-properties').textContent =
            stats.total_properties?.toLocaleString() || this.currentProperties.length.toLocaleString();

        const avgPrice = stats.price_statistics?.mean || 0;
        document.getElementById('avg-price').textContent = '₹' + this.formatPrice(avgPrice);

        document.getElementById('total-cities').textContent =
            stats.total_cities || new Set(this.currentProperties.map(p => p.city)).size;

        // Random trend for demo
        const trend = (Math.random() > 0.5 ? '+' : '-') + (Math.random() * 10).toFixed(1) + '%';
        document.getElementById('price-trend').textContent = trend;

        // Check if we're in the statistics section
        const statsSection = document.getElementById('stats');
        if (statsSection && statsSection.classList.contains('active')) {
            // Delay chart update slightly to ensure DOM is ready
            setTimeout(() => this.updateCharts(), 100);
        }
    }

    async fetchWithTimeout(url, timeout = 5000) {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), timeout);

        try {
            const response = await fetch(url, { signal: controller.signal });
            clearTimeout(timeoutId);
            return response;
        } catch (error) {
            clearTimeout(timeoutId);
            throw error;
        }
    }

    showLoading(show) {
        const loading = document.getElementById('loading');
        if (show) {
            loading.classList.remove('hidden');
        } else {
            loading.classList.add('hidden');
        }
    }

    showToast(message, type = 'info') {
        const container = document.getElementById('toast-container');

        const toast = document.createElement('div');
        toast.className = `toast ${type}`;

        const icon = type === 'success' ? 'check-circle' :
            type === 'error' ? 'exclamation-circle' :
                type === 'warning' ? 'exclamation-triangle' : 'info-circle';

        toast.innerHTML = `
            <i class="fas fa-${icon}"></i>
            <span>${message}</span>
        `;

        container.appendChild(toast);

        // Auto remove after 5 seconds
        setTimeout(() => {
            if (toast.parentNode) {
                toast.parentNode.removeChild(toast);
            }
        }, 5000);

        // Remove on click
        toast.addEventListener('click', () => {
            if (toast.parentNode) {
                toast.parentNode.removeChild(toast);
            }
        });
    }
}

// Initialize the application
let app;

document.addEventListener('DOMContentLoaded', () => {
    app = new PropertyFinder();
});