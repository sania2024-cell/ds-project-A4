#include "property_manager.h"
#include "csv_parser.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>

PropertyManager::PropertyManager() {}

PropertyManager::~PropertyManager() {}

void PropertyManager::addProperty(const Property &property)
{
    properties_.push_back(property);
    updateIndexMap();
}

void PropertyManager::removeProperty(int id)
{
    auto it = std::remove_if(properties_.begin(), properties_.end(),
                             [id](const Property &p)
                             { return p.getId() == id; });

    if (it != properties_.end())
    {
        properties_.erase(it, properties_.end());
        updateIndexMap();
    }
}

Property *PropertyManager::findProperty(int id)
{
    auto it = id_to_index_.find(id);
    if (it != id_to_index_.end() && it->second < properties_.size())
    {
        return &properties_[it->second];
    }
    return nullptr;
}

bool PropertyManager::loadFromCSV(const std::string &filename)
{
    CSVParser parser;
    auto csvData = parser.parseCSV(filename);

    if (csvData.empty())
    {
        std::cerr << "Error: Failed to load data from " << filename << std::endl;
        return false;
    }

    // Clear existing properties
    clearProperties();

    // Skip header row
    for (size_t i = 1; i < csvData.size(); ++i)
    {
        const auto &row = csvData[i];

        if (row.size() >= 9)
        { // Minimum required columns
            try
            {
                Property property;
                property.setId(std::stoi(row[0]));
                property.setCity(row[1]);
                property.setPrice(std::stod(row[2]));
                property.setBedrooms(std::stoi(row[3]));
                property.setBathrooms(std::stoi(row[4]));
                property.setSize(std::stod(row[5]));
                property.setType(row[6]);
                property.setLatitude(std::stod(row[7]));
                property.setLongitude(std::stod(row[8]));

                // Parse amenities if available
                if (row.size() > 9 && !row[9].empty())
                {
                    std::vector<std::string> amenities;
                    std::string amenitiesStr = row[9];
                    size_t pos = 0;
                    while ((pos = amenitiesStr.find(',')) != std::string::npos)
                    {
                        amenities.push_back(amenitiesStr.substr(0, pos));
                        amenitiesStr.erase(0, pos + 1);
                    }
                    if (!amenitiesStr.empty())
                    {
                        amenities.push_back(amenitiesStr);
                    }
                    property.setAmenities(amenities);
                }

                addProperty(property);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing row " << i << ": " << e.what() << std::endl;
                continue;
            }
        }
    }

    std::cout << "Loaded " << properties_.size() << " properties from " << filename << std::endl;
    return true;
}

bool PropertyManager::saveToCSV(const std::string &filename)
{
    CSVParser parser;
    std::vector<std::vector<std::string>> csvData;

    // Add header
    csvData.push_back({"ID", "City", "Price", "Bedrooms", "Bathrooms",
                       "Size", "Type", "Latitude", "Longitude", "Amenities", "PredictedPrice"});

    // Add property data
    for (const auto &property : properties_)
    {
        std::vector<std::string> row;
        row.push_back(std::to_string(property.getId()));
        row.push_back(property.getCity());
        row.push_back(std::to_string(property.getPrice()));
        row.push_back(std::to_string(property.getBedrooms()));
        row.push_back(std::to_string(property.getBathrooms()));
        row.push_back(std::to_string(property.getSize()));
        row.push_back(property.getType());
        row.push_back(std::to_string(property.getLatitude()));
        row.push_back(std::to_string(property.getLongitude()));

        // Combine amenities
        std::string amenitiesStr;
        const auto &amenities = property.getAmenities();
        for (size_t i = 0; i < amenities.size(); ++i)
        {
            amenitiesStr += amenities[i];
            if (i < amenities.size() - 1)
                amenitiesStr += ",";
        }
        row.push_back(amenitiesStr);
        row.push_back(std::to_string(property.getPredictedPrice()));

        csvData.push_back(row);
    }

    return parser.writeCSV(filename, csvData);
}

double PropertyManager::getAveragePrice() const
{
    if (properties_.empty())
        return 0.0;

    double sum = std::accumulate(properties_.begin(), properties_.end(), 0.0,
                                 [](double acc, const Property &p)
                                 { return acc + p.getPrice(); });

    return sum / properties_.size();
}

std::map<std::string, int> PropertyManager::getCityCounts() const
{
    std::map<std::string, int> cityCounts;
    for (const auto &property : properties_)
    {
        cityCounts[property.getCity()]++;
    }
    return cityCounts;
}

std::map<std::string, int> PropertyManager::getTypeCounts() const
{
    std::map<std::string, int> typeCounts;
    for (const auto &property : properties_)
    {
        typeCounts[property.getType()]++;
    }
    return typeCounts;
}

std::vector<Property> PropertyManager::searchProperties(const std::map<std::string, std::string> &filters)
{
    std::vector<Property> results;

    for (const auto &property : properties_)
    {
        if (property.matchesFilter(filters))
        {
            results.push_back(property);
        }
    }

    return results;
}

std::vector<Property> PropertyManager::getPropertiesByCity(const std::string &city)
{
    std::vector<Property> results;

    std::copy_if(properties_.begin(), properties_.end(), std::back_inserter(results),
                 [&city](const Property &p)
                 { return p.getCity() == city; });

    return results;
}

std::vector<Property> PropertyManager::getPropertiesByPriceRange(double minPrice, double maxPrice)
{
    std::vector<Property> results;

    std::copy_if(properties_.begin(), properties_.end(), std::back_inserter(results),
                 [minPrice, maxPrice](const Property &p)
                 {
                     return p.getPrice() >= minPrice && p.getPrice() <= maxPrice;
                 });

    return results;
}

std::vector<Property> PropertyManager::getPropertiesByBedrooms(int bedrooms)
{
    std::vector<Property> results;

    std::copy_if(properties_.begin(), properties_.end(), std::back_inserter(results),
                 [bedrooms](const Property &p)
                 { return p.getBedrooms() == bedrooms; });

    return results;
}

void PropertyManager::sortByPrice(bool ascending)
{
    if (ascending)
    {
        std::sort(properties_.begin(), properties_.end(),
                  [](const Property &a, const Property &b)
                  { return a.getPrice() < b.getPrice(); });
    }
    else
    {
        std::sort(properties_.begin(), properties_.end(),
                  [](const Property &a, const Property &b)
                  { return a.getPrice() > b.getPrice(); });
    }
    updateIndexMap();
}

void PropertyManager::sortBySize(bool ascending)
{
    if (ascending)
    {
        std::sort(properties_.begin(), properties_.end(),
                  [](const Property &a, const Property &b)
                  { return a.getSize() < b.getSize(); });
    }
    else
    {
        std::sort(properties_.begin(), properties_.end(),
                  [](const Property &a, const Property &b)
                  { return a.getSize() > b.getSize(); });
    }
    updateIndexMap();
}

void PropertyManager::sortByCity()
{
    std::sort(properties_.begin(), properties_.end(),
              [](const Property &a, const Property &b)
              { return a.getCity() < b.getCity(); });
    updateIndexMap();
}

void PropertyManager::printStatistics() const
{
    std::cout << "\n=== Property Statistics ===" << std::endl;
    std::cout << "Total Properties: " << properties_.size() << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average Price: ₹" << getAveragePrice() << std::endl;

    std::cout << "\nCity Distribution:" << std::endl;
    auto cityCounts = getCityCounts();
    for (const auto &pair : cityCounts)
    {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }

    std::cout << "\nProperty Type Distribution:" << std::endl;
    auto typeCounts = getTypeCounts();
    for (const auto &pair : typeCounts)
    {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
}

void PropertyManager::updateIndexMap()
{
    id_to_index_.clear();
    for (size_t i = 0; i < properties_.size(); ++i)
    {
        id_to_index_[properties_[i].getId()] = i;
    }
}