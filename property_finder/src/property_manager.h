#ifndef PROPERTY_MANAGER_H
#define PROPERTY_MANAGER_H

#include "property.h"
#include <vector>
#include <map>
#include <string>

/**
 * PropertyManager class for managing property collections
 */
class PropertyManager
{
public:
    PropertyManager();
    ~PropertyManager();

    // Property management
    void addProperty(const Property &property);
    void removeProperty(int id);
    Property *findProperty(int id);
    std::vector<Property> &getAllProperties() { return properties_; }

    // Data loading
    bool loadFromCSV(const std::string &filename);
    bool saveToCSV(const std::string &filename);

    // Statistics
    size_t getPropertyCount() const { return properties_.size(); }
    double getAveragePrice() const;
    std::map<std::string, int> getCityCounts() const;
    std::map<std::string, int> getTypeCounts() const;

    // Search and filter
    std::vector<Property> searchProperties(const std::map<std::string, std::string> &filters);
    std::vector<Property> getPropertiesByCity(const std::string &city);
    std::vector<Property> getPropertiesByPriceRange(double minPrice, double maxPrice);
    std::vector<Property> getPropertiesByBedrooms(int bedrooms);

    // Sorting
    void sortByPrice(bool ascending = true);
    void sortBySize(bool ascending = true);
    void sortByCity();

    // Utility
    void clearProperties() { properties_.clear(); }
    void printStatistics() const;

private:
    std::vector<Property> properties_;
    std::map<int, size_t> id_to_index_; // For fast lookup by ID

    void updateIndexMap();
};

#endif // PROPERTY_MANAGER_H