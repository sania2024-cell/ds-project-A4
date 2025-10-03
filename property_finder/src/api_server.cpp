#include "api_server.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

APIServer::APIServer(PropertyManager *pm, SearchEngine *se, MLPredictor *ml)
    : property_manager_(pm), search_engine_(se), ml_predictor_(ml),
      is_running_(false), port_(8080), enable_cors_(true), max_results_(50) {}

APIServer::~APIServer()
{
    if (is_running_)
    {
        stop();
    }
}

bool APIServer::start(int port)
{
    port_ = port;
    is_running_ = true;

    std::cout << "API Server started on port " << port_ << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET /search?city=...&min_price=...&max_price=..." << std::endl;
    std::cout << "  GET /predict?property_id=..." << std::endl;
    std::cout << "  GET /recommend?property_id=..." << std::endl;
    std::cout << "  GET /nearby?lat=...&lon=...&radius=..." << std::endl;
    std::cout << "  GET /stats" << std::endl;

    return true;
}

void APIServer::stop()
{
    is_running_ = false;
    std::cout << "API Server stopped" << std::endl;
}

std::string APIServer::handleRequest(const std::string &path, const std::map<std::string, std::string> &params)
{
    std::string headers = enable_cors_ ? formatCORSHeaders() : "";

    try
    {
        if (path == "/search")
        {
            return headers + handleSearch(params);
        }
        else if (path == "/predict")
        {
            return headers + handlePredict(params);
        }
        else if (path == "/recommend")
        {
            return headers + handleRecommend(params);
        }
        else if (path == "/nearby")
        {
            return headers + handleNearby(params);
        }
        else if (path == "/stats")
        {
            return headers + handleStats(params);
        }
        else
        {
            return headers + errorResponse("Endpoint not found: " + path);
        }
    }
    catch (const std::exception &e)
    {
        return headers + errorResponse("Server error: " + std::string(e.what()));
    }
}

std::string APIServer::handleSearch(const std::map<std::string, std::string> &params)
{
    auto results = search_engine_->search(property_manager_->getAllProperties(), params);

    // Limit results
    if (results.size() > static_cast<size_t>(max_results_))
    {
        results.resize(max_results_);
    }

    return propertiesToJSON(results);
}

std::string APIServer::handlePredict(const std::map<std::string, std::string> &params)
{
    auto it = params.find("property_id");
    if (it == params.end())
    {
        return errorResponse("Missing property_id parameter");
    }

    try
    {
        int property_id = std::stoi(it->second);
        Property *property = property_manager_->findProperty(property_id);

        if (!property)
        {
            return errorResponse("Property not found: " + it->second);
        }

        double predicted_price = ml_predictor_->predictPrice(*property);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "{\n";
        oss << "  \"property_id\": " << property_id << ",\n";
        oss << "  \"actual_price\": " << property->getPrice() << ",\n";
        oss << "  \"predicted_price\": " << predicted_price << ",\n";
        oss << "  \"difference\": " << (predicted_price - property->getPrice()) << ",\n";
        oss << "  \"accuracy\": " << (100.0 - std::fabs(static_cast<double>(predicted_price - property->getPrice())) / property->getPrice() * 100.0) << "%\"\n";
        oss << "}";

        return oss.str();
    }
    catch (const std::exception &e)
    {
        return errorResponse("Invalid property_id: " + it->second);
    }
}

std::string APIServer::handleRecommend(const std::map<std::string, std::string> &params)
{
    auto it = params.find("property_id");
    if (it == params.end())
    {
        return errorResponse("Missing property_id parameter");
    }

    try
    {
        int property_id = std::stoi(it->second);
        Property *target_property = property_manager_->findProperty(property_id);

        if (!target_property)
        {
            return errorResponse("Property not found: " + it->second);
        }

        int max_results = 5;
        auto max_it = params.find("max_results");
        if (max_it != params.end())
        {
            max_results = std::stoi(max_it->second);
        }

        auto recommendations = search_engine_->recommendSimilar(
            property_manager_->getAllProperties(), *target_property, max_results);

        return propertiesToJSON(recommendations);
    }
    catch (const std::exception &e)
    {
        return errorResponse("Invalid property_id: " + it->second);
    }
}

std::string APIServer::handleNearby(const std::map<std::string, std::string> &params)
{
    auto lat_it = params.find("lat");
    auto lon_it = params.find("lon");
    auto radius_it = params.find("radius");

    if (lat_it == params.end() || lon_it == params.end())
    {
        return errorResponse("Missing lat/lon parameters");
    }

    try
    {
        double latitude = std::stod(lat_it->second);
        double longitude = std::stod(lon_it->second);
        double radius = radius_it != params.end() ? std::stod(radius_it->second) : 10.0; // Default 10km

        auto nearby_properties = search_engine_->searchNearby(
            property_manager_->getAllProperties(), latitude, longitude, radius);

        // Limit results
        if (nearby_properties.size() > static_cast<size_t>(max_results_))
        {
            nearby_properties.resize(max_results_);
        }

        return propertiesToJSON(nearby_properties);
    }
    catch (const std::exception &e)
    {
        return errorResponse("Invalid coordinates: " + std::string(e.what()));
    }
}

std::string APIServer::handleStats(const std::map<std::string, std::string> &params)
{
    auto &properties = property_manager_->getAllProperties();
    auto price_stats = search_engine_->calculatePriceStatistics(properties);
    auto amenity_counts = search_engine_->getPopularAmenities(properties);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n";
    oss << "  \"total_properties\": " << properties.size() << ",\n";
    oss << "  \"price_statistics\": {\n";

    bool first = true;
    for (const auto &pair : price_stats)
    {
        if (!first)
            oss << ",\n";
        oss << "    \"" << pair.first << "\": " << pair.second;
        first = false;
    }

    oss << "\n  },\n";
    oss << "  \"popular_amenities\": {\n";

    first = true;
    int count = 0;
    for (const auto &pair : amenity_counts)
    {
        if (count >= 10)
            break; // Top 10 amenities
        if (!first)
            oss << ",\n";
        oss << "    \"" << escapeJSON(pair.first) << "\": " << pair.second;
        first = false;
        count++;
    }

    oss << "\n  }\n";
    oss << "}";

    return oss.str();
}

std::string APIServer::propertiesToJSON(const std::vector<Property> &properties)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n";
    oss << "  \"count\": " << properties.size() << ",\n";
    oss << "  \"properties\": [\n";

    for (size_t i = 0; i < properties.size(); ++i)
    {
        const auto &property = properties[i];

        oss << "    {\n";
        oss << "      \"id\": " << property.getId() << ",\n";
        oss << "      \"city\": \"" << escapeJSON(property.getCity()) << "\",\n";
        oss << "      \"price\": " << property.getPrice() << ",\n";
        oss << "      \"bedrooms\": " << property.getBedrooms() << ",\n";
        oss << "      \"bathrooms\": " << property.getBathrooms() << ",\n";
        oss << "      \"size\": " << property.getSize() << ",\n";
        oss << "      \"type\": \"" << escapeJSON(property.getType()) << "\",\n";
        oss << "      \"latitude\": " << property.getLatitude() << ",\n";
        oss << "      \"longitude\": " << property.getLongitude() << ",\n";

        if (property.getPredictedPrice() > 0)
        {
            oss << "      \"predicted_price\": " << property.getPredictedPrice() << ",\n";
        }

        oss << "      \"amenities\": [";
        const auto &amenities = property.getAmenities();
        for (size_t j = 0; j < amenities.size(); ++j)
        {
            oss << "\"" << escapeJSON(amenities[j]) << "\"";
            if (j < amenities.size() - 1)
                oss << ", ";
        }
        oss << "]\n";

        oss << "    }";
        if (i < properties.size() - 1)
            oss << ",";
        oss << "\n";
    }

    oss << "  ]\n";
    oss << "}";

    return oss.str();
}

std::string APIServer::statsToJSON(const std::map<std::string, double> &stats)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n";

    bool first = true;
    for (const auto &pair : stats)
    {
        if (!first)
            oss << ",\n";
        oss << "  \"" << pair.first << "\": " << pair.second;
        first = false;
    }

    oss << "\n}";
    return oss.str();
}

std::string APIServer::errorResponse(const std::string &message)
{
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"error\": true,\n";
    oss << "  \"message\": \"" << escapeJSON(message) << "\"\n";
    oss << "}";
    return oss.str();
}

std::map<std::string, std::string> APIServer::parseQueryParams(const std::string &query)
{
    std::map<std::string, std::string> params;
    std::istringstream iss(query);
    std::string pair;

    while (std::getline(iss, pair, '&'))
    {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos)
        {
            std::string key = urlDecode(pair.substr(0, eq_pos));
            std::string value = urlDecode(pair.substr(eq_pos + 1));
            params[key] = value;
        }
    }

    return params;
}

std::string APIServer::urlDecode(const std::string &str)
{
    std::string result;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '%' && i + 2 < str.length())
        {
            int hex_value;
            std::istringstream hex_stream(str.substr(i + 1, 2));
            hex_stream >> std::hex >> hex_value;
            result += static_cast<char>(hex_value);
            i += 2;
        }
        else if (str[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += str[i];
        }
    }
    return result;
}

std::string APIServer::escapeJSON(const std::string &str)
{
    std::string result;
    for (char c : str)
    {
        switch (c)
        {
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            result += c;
            break;
        }
    }
    return result;
}

std::string APIServer::formatCORSHeaders()
{
    return "Access-Control-Allow-Origin: *\n"
           "Access-Control-Allow-Methods: GET, POST, OPTIONS\n"
           "Access-Control-Allow-Headers: Content-Type\n"
           "Content-Type: application/json\n\n";
}