#include "search_engine.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SearchEngine::SearchEngine() {}

SearchEngine::~SearchEngine() {}

std::vector<Property> SearchEngine::search(const std::vector<Property> &properties,
                                           const std::map<std::string, std::string> &filters)
{
    std::vector<Property> results;

    for (const auto &property : properties)
    {
        if (property.matchesFilter(filters))
        {
            results.push_back(property);
        }
    }

    return results;
}

std::vector<Property> SearchEngine::searchByKeywords(const std::vector<Property> &properties,
                                                     const std::string &keywords)
{
    std::vector<Property> results;

    for (const auto &property : properties)
    {
        if (matchesKeywords(property, keywords))
        {
            results.push_back(property);
        }
    }

    return results;
}

std::vector<Property> SearchEngine::searchNearby(const std::vector<Property> &properties,
                                                 double latitude, double longitude, double radius_km)
{
    std::vector<Property> results;

    for (const auto &property : properties)
    {
        double distance = calculateDistance(latitude, longitude,
                                            property.getLatitude(), property.getLongitude());
        if (distance <= radius_km)
        {
            results.push_back(property);
        }
    }

    return results;
}

std::vector<Property> SearchEngine::recommendSimilar(const std::vector<Property> &properties,
                                                     const Property &target_property, int max_results)
{
    std::vector<std::pair<double, Property>> scored_properties;

    for (const auto &property : properties)
    {
        if (property.getId() != target_property.getId())
        {
            double similarity = calculateSimilarity(target_property, property);
            scored_properties.emplace_back(similarity, property);
        }
    }

    // Sort by similarity (descending)
    std::sort(scored_properties.begin(), scored_properties.end(),
              [](const auto &a, const auto &b)
              { return a.first > b.first; });

    std::vector<Property> results;
    int count = std::min(max_results, static_cast<int>(scored_properties.size()));

    for (int i = 0; i < count; ++i)
    {
        results.push_back(scored_properties[i].second);
    }

    return results;
}

std::vector<Property> SearchEngine::recommendByBudget(const std::vector<Property> &properties,
                                                      double budget, double tolerance)
{
    std::vector<Property> results;
    double min_price = budget * (1.0 - tolerance);
    double max_price = budget * (1.0 + tolerance);

    for (const auto &property : properties)
    {
        if (property.getPrice() >= min_price && property.getPrice() <= max_price)
        {
            results.push_back(property);
        }
    }

    // Sort by proximity to budget
    std::sort(results.begin(), results.end(),
              [budget](const Property &a, const Property &b)
              {
                  return std::abs(a.getPrice() - budget) < std::abs(b.getPrice() - budget);
              });

    return results;
}

void SearchEngine::sortProperties(std::vector<Property> &properties, SortBy sort_by,
                                  double ref_latitude, double ref_longitude)
{
    switch (sort_by)
    {
    case SortBy::PRICE_ASC:
        std::sort(properties.begin(), properties.end(),
                  [](const Property &a, const Property &b)
                  { return a.getPrice() < b.getPrice(); });
        break;

    case SortBy::PRICE_DESC:
        std::sort(properties.begin(), properties.end(),
                  [](const Property &a, const Property &b)
                  { return a.getPrice() > b.getPrice(); });
        break;

    case SortBy::SIZE_ASC:
        std::sort(properties.begin(), properties.end(),
                  [](const Property &a, const Property &b)
                  { return a.getSize() < b.getSize(); });
        break;

    case SortBy::SIZE_DESC:
        std::sort(properties.begin(), properties.end(),
                  [](const Property &a, const Property &b)
                  { return a.getSize() > b.getSize(); });
        break;

    case SortBy::BEDROOMS_ASC:
        std::sort(properties.begin(), properties.end(),
                  [](const Property &a, const Property &b)
                  { return a.getBedrooms() < b.getBedrooms(); });
        break;

    case SortBy::BEDROOMS_DESC:
        std::sort(properties.begin(), properties.end(),
                  [](const Property &a, const Property &b)
                  { return a.getBedrooms() > b.getBedrooms(); });
        break;

    case SortBy::DISTANCE:
        std::sort(properties.begin(), properties.end(),
                  [this, ref_latitude, ref_longitude](const Property &a, const Property &b)
                  {
                      double dist_a = calculateDistance(ref_latitude, ref_longitude, a.getLatitude(), a.getLongitude());
                      double dist_b = calculateDistance(ref_latitude, ref_longitude, b.getLatitude(), b.getLongitude());
                      return dist_a < dist_b;
                  });
        break;

    default:
        break;
    }
}

std::map<std::string, double> SearchEngine::calculatePriceStatistics(const std::vector<Property> &properties)
{
    std::map<std::string, double> stats;

    if (properties.empty())
    {
        return stats;
    }

    std::vector<double> prices;
    for (const auto &property : properties)
    {
        prices.push_back(property.getPrice());
    }

    std::sort(prices.begin(), prices.end());

    // Calculate statistics
    double sum = 0.0;
    for (double price : prices)
    {
        sum += price;
    }

    stats["count"] = static_cast<double>(prices.size());
    stats["mean"] = sum / prices.size();
    stats["min"] = prices.front();
    stats["max"] = prices.back();
    stats["median"] = (prices.size() % 2 == 0) ? (prices[prices.size() / 2 - 1] + prices[prices.size() / 2]) / 2.0 : prices[prices.size() / 2];

    // Calculate standard deviation
    double variance = 0.0;
    for (double price : prices)
    {
        variance += std::pow(price - stats["mean"], 2);
    }
    stats["std_dev"] = std::sqrt(variance / prices.size());

    return stats;
}

std::map<std::string, int> SearchEngine::getPopularAmenities(const std::vector<Property> &properties)
{
    std::map<std::string, int> amenity_counts;

    for (const auto &property : properties)
    {
        const auto &amenities = property.getAmenities();
        for (const auto &amenity : amenities)
        {
            amenity_counts[amenity]++;
        }
    }

    return amenity_counts;
}

double SearchEngine::calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    const double R = 6371.0; // Earth's radius in kilometers

    double lat1_rad = lat1 * M_PI / 180.0;
    double lon1_rad = lon1 * M_PI / 180.0;
    double lat2_rad = lat2 * M_PI / 180.0;
    double lon2_rad = lon2 * M_PI / 180.0;

    double dlat = lat2_rad - lat1_rad;
    double dlon = lon2_rad - lon1_rad;

    double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
               std::cos(lat1_rad) * std::cos(lat2_rad) *
                   std::sin(dlon / 2) * std::sin(dlon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return R * c;
}

double SearchEngine::calculateSimilarity(const Property &p1, const Property &p2)
{
    double similarity = 0.0;

    // City similarity (exact match gets high score)
    if (p1.getCity() == p2.getCity())
    {
        similarity += 0.3;
    }

    // Property type similarity
    if (p1.getType() == p2.getType())
    {
        similarity += 0.2;
    }

    // Bedrooms similarity
    int bedroom_diff = std::abs(p1.getBedrooms() - p2.getBedrooms());
    similarity += (bedroom_diff == 0) ? 0.2 : (bedroom_diff == 1) ? 0.1
                                                                  : 0.0;

    // Price similarity (normalized)
    double price_ratio = std::min(p1.getPrice(), p2.getPrice()) / std::max(p1.getPrice(), p2.getPrice());
    similarity += 0.2 * price_ratio;

    // Size similarity (normalized)
    double size_ratio = std::min(p1.getSize(), p2.getSize()) / std::max(p1.getSize(), p2.getSize());
    similarity += 0.1 * size_ratio;

    return similarity;
}

bool SearchEngine::matchesKeywords(const Property &property, const std::string &keywords)
{
    std::vector<std::string> tokens = tokenizeKeywords(keywords);

    for (const auto &token : tokens)
    {
        std::string lower_token = token;
        std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(), ::tolower);

        // Check city
        std::string lower_city = property.getCity();
        std::transform(lower_city.begin(), lower_city.end(), lower_city.begin(), ::tolower);
        if (lower_city.find(lower_token) != std::string::npos)
        {
            return true;
        }

        // Check type
        std::string lower_type = property.getType();
        std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(), ::tolower);
        if (lower_type.find(lower_token) != std::string::npos)
        {
            return true;
        }

        // Check amenities
        for (const auto &amenity : property.getAmenities())
        {
            std::string lower_amenity = amenity;
            std::transform(lower_amenity.begin(), lower_amenity.end(), lower_amenity.begin(), ::tolower);
            if (lower_amenity.find(lower_token) != std::string::npos)
            {
                return true;
            }
        }
    }

    return false;
}

std::vector<std::string> SearchEngine::tokenizeKeywords(const std::string &keywords)
{
    std::vector<std::string> tokens;
    std::istringstream iss(keywords);
    std::string token;

    while (iss >> token)
    {
        tokens.push_back(token);
    }

    return tokens;
}