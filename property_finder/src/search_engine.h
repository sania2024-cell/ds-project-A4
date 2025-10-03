#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "property.h"
#include <vector>
#include <map>
#include <string>

/**
 * SearchEngine class for advanced property searching and filtering
 */
class SearchEngine
{
public:
    SearchEngine();
    ~SearchEngine();

    // Search methods
    std::vector<Property> search(const std::vector<Property> &properties,
                                 const std::map<std::string, std::string> &filters);

    std::vector<Property> searchByKeywords(const std::vector<Property> &properties,
                                           const std::string &keywords);

    std::vector<Property> searchNearby(const std::vector<Property> &properties,
                                       double latitude, double longitude, double radius_km);

    // Recommendation methods
    std::vector<Property> recommendSimilar(const std::vector<Property> &properties,
                                           const Property &target_property, int max_results = 5);

    std::vector<Property> recommendByBudget(const std::vector<Property> &properties,
                                            double budget, double tolerance = 0.1);

    // Sorting methods
    enum class SortBy
    {
        PRICE_ASC,
        PRICE_DESC,
        SIZE_ASC,
        SIZE_DESC,
        BEDROOMS_ASC,
        BEDROOMS_DESC,
        RELEVANCE,
        DISTANCE
    };

    void sortProperties(std::vector<Property> &properties, SortBy sort_by,
                        double ref_latitude = 0.0, double ref_longitude = 0.0);

    // Analytics
    std::map<std::string, double> calculatePriceStatistics(const std::vector<Property> &properties);
    std::map<std::string, int> getPopularAmenities(const std::vector<Property> &properties);

private:
    // Helper methods
    double calculateDistance(double lat1, double lon1, double lat2, double lon2);
    double calculateSimilarity(const Property &p1, const Property &p2);
    bool matchesKeywords(const Property &property, const std::string &keywords);
    std::vector<std::string> tokenizeKeywords(const std::string &keywords);
};

#endif // SEARCH_ENGINE_H