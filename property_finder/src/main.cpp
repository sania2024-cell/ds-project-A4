#include "property_manager.h"
#include "search_engine.h"
#include "ml_predictor.h"
#include "api_server.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <chrono>
#include <cmath>

void printWelcomeMessage()
{
    std::cout << "==========================================" << std::endl;
    std::cout << "   C++ Property Finder with ML & Maps    " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Smart property search with AI predictions" << std::endl;
    std::cout << "==========================================" << std::endl
              << std::endl;
}

void printMenu()
{
    std::cout << "\n=== Main Menu ===" << std::endl;
    std::cout << "1. Load properties from CSV" << std::endl;
    std::cout << "2. Search properties" << std::endl;
    std::cout << "3. Train ML model" << std::endl;
    std::cout << "4. Predict property price" << std::endl;
    std::cout << "5. Get recommendations" << std::endl;
    std::cout << "6. Find nearby properties" << std::endl;
    std::cout << "7. View statistics" << std::endl;
    std::cout << "8. Start API server" << std::endl;
    std::cout << "9. Export results to CSV" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Enter your choice: ";
}

void handleLoadProperties(PropertyManager &pm)
{
    std::string filename;
    std::cout << "Enter CSV filename (e.g., data/properties.csv): ";
    std::getline(std::cin, filename);

    Utils::Timer timer;
    timer.start();

    if (pm.loadFromCSV(filename))
    {
        timer.stop();
        std::cout << "Successfully loaded " << pm.getPropertyCount() << " properties in "
                  << Utils::formatDuration(timer.elapsed()) << std::endl;
    }
    else
    {
        std::cout << "Failed to load properties from " << filename << std::endl;
    }
}

void handleSearchProperties(PropertyManager &pm, SearchEngine &se)
{
    if (pm.getPropertyCount() == 0)
    {
        std::cout << "No properties loaded. Please load properties first." << std::endl;
        return;
    }

    std::map<std::string, std::string> filters;
    std::string input;

    std::cout << "Enter search filters (press Enter to skip):" << std::endl;

    std::cout << "City: ";
    std::getline(std::cin, input);
    if (!input.empty())
        filters["city"] = input;

    std::cout << "Minimum price: ";
    std::getline(std::cin, input);
    if (!input.empty())
        filters["min_price"] = input;

    std::cout << "Maximum price: ";
    std::getline(std::cin, input);
    if (!input.empty())
        filters["max_price"] = input;

    std::cout << "Bedrooms: ";
    std::getline(std::cin, input);
    if (!input.empty())
        filters["bedrooms"] = input;

    Utils::Timer timer;
    timer.start();

    auto results = se.search(pm.getAllProperties(), filters);

    timer.stop();

    std::cout << "\nFound " << results.size() << " properties in "
              << Utils::formatDuration(timer.elapsed()) << std::endl;

    if (results.size() > 0)
    {
        int max_display = std::min(10, static_cast<int>(results.size()));
        std::cout << "\nShowing first " << max_display << " results:" << std::endl;

        for (int i = 0; i < max_display; ++i)
        {
            std::cout << "\n--- Property " << (i + 1) << " ---" << std::endl;
            std::cout << results[i].toString() << std::endl;
        }
    }
}

void handleTrainModel(PropertyManager &pm, MLPredictor &ml)
{
    if (pm.getPropertyCount() == 0)
    {
        std::cout << "No properties loaded. Please load properties first." << std::endl;
        return;
    }

    std::cout << "Training ML model with " << pm.getPropertyCount() << " properties..." << std::endl;

    Utils::Timer timer;
    timer.start();

    if (ml.trainModel(pm.getAllProperties()))
    {
        timer.stop();
        std::cout << "Model training completed in " << Utils::formatDuration(timer.elapsed()) << std::endl;

        // Calculate and display model metrics
        auto metrics = ml.getModelMetrics();
        std::cout << "\nModel Metrics:" << std::endl;
        for (const auto &metric : metrics)
        {
            std::cout << "  " << metric.first << ": " << metric.second << std::endl;
        }
    }
    else
    {
        std::cout << "Model training failed!" << std::endl;
    }
}

void handlePredictPrice(PropertyManager &pm, MLPredictor &ml)
{
    if (pm.getPropertyCount() == 0)
    {
        std::cout << "No properties loaded. Please load properties first." << std::endl;
        return;
    }

    std::string input;
    std::cout << "Enter property ID to predict price for: ";
    std::getline(std::cin, input);

    try
    {
        int property_id = std::stoi(input);
        Property *property = pm.findProperty(property_id);

        if (property)
        {
            double predicted_price = ml.predictPrice(*property);
            double actual_price = property->getPrice();
            double accuracy = 100.0 - std::fabs(predicted_price - actual_price) / actual_price * 100.0;

            std::cout << "\n=== Price Prediction ===" << std::endl;
            std::cout << "Property ID: " << property_id << std::endl;
            std::cout << "Actual Price: ₹" << actual_price << std::endl;
            std::cout << "Predicted Price: ₹" << predicted_price << std::endl;
            std::cout << "Difference: ₹" << (predicted_price - actual_price) << std::endl;
            std::cout << "Accuracy: " << Utils::roundToDecimalPlaces(accuracy, 2) << "%" << std::endl;
        }
        else
        {
            std::cout << "Property with ID " << property_id << " not found." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Invalid property ID: " << input << std::endl;
    }
}

void handleRecommendations(PropertyManager &pm, SearchEngine &se)
{
    if (pm.getPropertyCount() == 0)
    {
        std::cout << "No properties loaded. Please load properties first." << std::endl;
        return;
    }

    std::string input;
    std::cout << "Enter property ID for recommendations: ";
    std::getline(std::cin, input);

    try
    {
        int property_id = std::stoi(input);
        Property *target_property = pm.findProperty(property_id);

        if (target_property)
        {
            auto recommendations = se.recommendSimilar(pm.getAllProperties(), *target_property, 5);

            std::cout << "\n=== Similar Properties ===" << std::endl;
            std::cout << "Based on property ID: " << property_id << std::endl;

            for (size_t i = 0; i < recommendations.size(); ++i)
            {
                std::cout << "\n--- Recommendation " << (i + 1) << " ---" << std::endl;
                std::cout << recommendations[i].toString() << std::endl;
            }
        }
        else
        {
            std::cout << "Property with ID " << property_id << " not found." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Invalid property ID: " << input << std::endl;
    }
}

void handleNearbySearch(PropertyManager &pm, SearchEngine &se)
{
    if (pm.getPropertyCount() == 0)
    {
        std::cout << "No properties loaded. Please load properties first." << std::endl;
        return;
    }

    std::string lat_str, lon_str, radius_str;

    std::cout << "Enter latitude: ";
    std::getline(std::cin, lat_str);

    std::cout << "Enter longitude: ";
    std::getline(std::cin, lon_str);

    std::cout << "Enter search radius (km, default 10): ";
    std::getline(std::cin, radius_str);

    try
    {
        double latitude = std::stod(lat_str);
        double longitude = std::stod(lon_str);
        double radius = radius_str.empty() ? 10.0 : std::stod(radius_str);

        auto nearby_properties = se.searchNearby(pm.getAllProperties(), latitude, longitude, radius);

        std::cout << "\nFound " << nearby_properties.size() << " properties within "
                  << radius << "km of (" << latitude << ", " << longitude << ")" << std::endl;

        int max_display = std::min(10, static_cast<int>(nearby_properties.size()));

        for (int i = 0; i < max_display; ++i)
        {
            std::cout << "\n--- Property " << (i + 1) << " ---" << std::endl;
            std::cout << nearby_properties[i].toString() << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Invalid coordinates or radius." << std::endl;
    }
}

void handleStatistics(PropertyManager &pm, SearchEngine &se)
{
    if (pm.getPropertyCount() == 0)
    {
        std::cout << "No properties loaded. Please load properties first." << std::endl;
        return;
    }

    pm.printStatistics();

    auto price_stats = se.calculatePriceStatistics(pm.getAllProperties());
    auto amenities = se.getPopularAmenities(pm.getAllProperties());

    std::cout << "\nPrice Analysis:" << std::endl;
    for (const auto &stat : price_stats)
    {
        std::cout << "  " << stat.first << ": " << Utils::roundToDecimalPlaces(stat.second, 2) << std::endl;
    }

    std::cout << "\nTop Amenities:" << std::endl;
    int count = 0;
    for (const auto &amenity : amenities)
    {
        if (count >= 10)
            break;
        std::cout << "  " << amenity.first << ": " << amenity.second << std::endl;
        count++;
    }
}

void handleAPIServer(PropertyManager &pm, SearchEngine &se, MLPredictor &ml)
{
    APIServer server(&pm, &se, &ml);

    std::string port_str;
    std::cout << "Enter port number (default 8080): ";
    std::getline(std::cin, port_str);

    int port = port_str.empty() ? 8080 : std::stoi(port_str);

    if (server.start(port))
    {
        std::cout << "API server running. Press Enter to stop..." << std::endl;
        std::cout << "\nExample requests:" << std::endl;
        std::cout << "  http://localhost:" << port << "/search?city=Mumbai&min_price=1000000" << std::endl;
        std::cout << "  http://localhost:" << port << "/stats" << std::endl;

        std::cin.get();
        server.stop();
    }
}

void handleExportCSV(PropertyManager &pm)
{
    if (pm.getPropertyCount() == 0)
    {
        std::cout << "No properties loaded. Please load properties first." << std::endl;
        return;
    }

    std::string filename;
    std::cout << "Enter output CSV filename: ";
    std::getline(std::cin, filename);

    if (pm.saveToCSV(filename))
    {
        std::cout << "Properties exported to " << filename << std::endl;
    }
    else
    {
        std::cout << "Failed to export properties to " << filename << std::endl;
    }
}

int main()
{
    printWelcomeMessage();

    // Initialize components
    PropertyManager property_manager;
    SearchEngine search_engine;
    MLPredictor ml_predictor;

    std::string input;

    while (true)
    {
        printMenu();
        std::getline(std::cin, input);

        if (input.empty())
            continue;

        int choice = std::stoi(input);

        switch (choice)
        {
        case 1:
            handleLoadProperties(property_manager);
            break;
        case 2:
            handleSearchProperties(property_manager, search_engine);
            break;
        case 3:
            handleTrainModel(property_manager, ml_predictor);
            break;
        case 4:
            handlePredictPrice(property_manager, ml_predictor);
            break;
        case 5:
            handleRecommendations(property_manager, search_engine);
            break;
        case 6:
            handleNearbySearch(property_manager, search_engine);
            break;
        case 7:
            handleStatistics(property_manager, search_engine);
            break;
        case 8:
            handleAPIServer(property_manager, search_engine, ml_predictor);
            break;
        case 9:
            handleExportCSV(property_manager);
            break;
        case 0:
            std::cout << "Thank you for using Property Finder!" << std::endl;
            return 0;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }

    return 0;
}