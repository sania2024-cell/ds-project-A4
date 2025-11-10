// Charts utilities for Property Finder

class ChartManager {
    static cityChart = null;
    static typeChart = null;

    static initializeCharts() {
        if (typeof Chart === 'undefined') {
            console.error('Chart.js is not loaded');
            return;
        }
        // Initialize empty charts
        const cityCtx = document.getElementById('city-chart');
        const typeCtx = document.getElementById('type-chart');
        
        if (cityCtx && !this.cityChart) {
            this.renderCityChart([]);
        }
        if (typeCtx && !this.typeChart) {
            this.renderTypeChart([]);
        }
    }

    static renderCityChart(properties) {
        const cityData = ChartManager.getCityDistribution(properties);
        const ctx = document.getElementById('city-chart').getContext('2d');
        
        if (window.cityChart) {
            window.cityChart.destroy();
        }

        window.cityChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: cityData.labels,
                datasets: [{
                    label: 'Properties per City',
                    data: cityData.data,
                    backgroundColor: 'rgba(54, 162, 235, 0.8)',
                    borderColor: 'rgba(54, 162, 235, 1)',
                    borderWidth: 1
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Number of Properties'
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Cities'
                        }
                    }
                }
            }
        });
    }

    static renderTypeChart(properties) {
        const typeData = ChartManager.getTypeDistribution(properties);
        const ctx = document.getElementById('type-chart').getContext('2d');
        
        if (window.typeChart) {
            window.typeChart.destroy();
        }

        window.typeChart = new Chart(ctx, {
            type: 'doughnut',
            data: {
                labels: typeData.labels,
                datasets: [{
                    data: typeData.data,
                    backgroundColor: [
                        'rgba(255, 99, 132, 0.8)',
                        'rgba(54, 162, 235, 0.8)',
                        'rgba(255, 206, 86, 0.8)',
                        'rgba(75, 192, 192, 0.8)',
                    ],
                    borderColor: [
                        'rgba(255, 99, 132, 1)',
                        'rgba(54, 162, 235, 1)',
                        'rgba(255, 206, 86, 1)',
                        'rgba(75, 192, 192, 1)',
                    ],
                    borderWidth: 1
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        position: 'right'
                    },
                    title: {
                        display: true,
                        text: 'Property Types'
                    }
                }
            }
        });
    }

    static getCityDistribution(properties) {
        const cityCounts = properties.reduce((acc, prop) => {
            acc[prop.city] = (acc[prop.city] || 0) + 1;
            return acc;
        }, {});

        return {
            labels: Object.keys(cityCounts),
            data: Object.values(cityCounts)
        };
    }

    static getTypeDistribution(properties) {
        const typeCounts = properties.reduce((acc, prop) => {
            acc[prop.type] = (acc[prop.type] || 0) + 1;
            return acc;
        }, {});

        return {
            labels: Object.keys(typeCounts),
            data: Object.values(typeCounts)
        };
    }
}

// Initialize charts when the page loads
document.addEventListener('DOMContentLoaded', () => {
    ChartManager.initializeCharts();
    // Add event listener for Chart.js loaded
    document.addEventListener('chartjs-loaded', () => {
        // Initial chart render if we're on the stats page
        const statsSection = document.getElementById('stats');
        if (statsSection.classList.contains('active')) {
            window.propertyFinder?.loadStatistics();
        }
    });
});