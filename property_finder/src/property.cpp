#include "property.h"
#include <sstream>
#include <iomanip>

Property::Property() : id_(0), price_(0.0), bedrooms_(0), bathrooms_(0),
                       size_(0.0), latitude_(0.0), longitude_(0.0), predicted_price_(0.0) {}

Property::Property(int id, const std::string &city, double price, int bedrooms,
                   int bathrooms, double size, const std::string &type,
                   double latitude, double longitude)
    : id_(id), city_(city), price_(price), bedrooms_(bedrooms),
      bathrooms_(bathrooms), size_(size), type_(type),
      latitude_(latitude), longitude_(longitude), predicted_price_(0.0) {}

std::string Property::toString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Property ID: " << id_ << "\n"
        << "City: " << city_ << "\n"
        << "Price: ₹" << price_ << "\n"
        << "Bedrooms: " << bedrooms_ << "\n"
        << "Bathrooms: " << bathrooms_ << "\n"
        << "Size: " << size_ << " sq ft\n"
        << "Type: " << type_ << "\n"
        << "Location: (" << latitude_ << ", " << longitude_ << ")\n";

    if (predicted_price_ > 0)
    {
        oss << "Predicted Price: ₹" << predicted_price_ << "\n";
    }

    if (!amenities_.empty())
    {
        oss << "Amenities: ";
        for (size_t i = 0; i < amenities_.size(); ++i)
        {
            oss << amenities_[i];
            if (i < amenities_.size() - 1)
                oss << ", ";
        }
        oss << "\n";
    }

    return oss.str();
}

std::map<std::string, std::string> Property::toMap() const
{
    std::map<std::string, std::string> propertyMap;

    propertyMap["id"] = std::to_string(id_);
    propertyMap["city"] = city_;
    propertyMap["price"] = std::to_string(price_);
    propertyMap["bedrooms"] = std::to_string(bedrooms_);
    propertyMap["bathrooms"] = std::to_string(bathrooms_);
    propertyMap["size"] = std::to_string(size_);
    propertyMap["type"] = type_;
    propertyMap["latitude"] = std::to_string(latitude_);
    propertyMap["longitude"] = std::to_string(longitude_);
    propertyMap["predicted_price"] = std::to_string(predicted_price_);

    // Combine amenities into a single string
    std::ostringstream amenitiesStr;
    for (size_t i = 0; i < amenities_.size(); ++i)
    {
        amenitiesStr << amenities_[i];
        if (i < amenities_.size() - 1)
            amenitiesStr << ",";
    }
    propertyMap["amenities"] = amenitiesStr.str();

    return propertyMap;
}

bool Property::matchesFilter(const std::map<std::string, std::string> &filters) const
{
    for (const auto &filter : filters)
    {
        const std::string &key = filter.first;
        const std::string &value = filter.second;

        if (key == "city" && city_ != value)
        {
            return false;
        }
        else if (key == "min_price" && price_ < std::stod(value))
        {
            return false;
        }
        else if (key == "max_price" && price_ > std::stod(value))
        {
            return false;
        }
        else if (key == "bedrooms" && bedrooms_ != std::stoi(value))
        {
            return false;
        }
        else if (key == "bathrooms" && bathrooms_ != std::stoi(value))
        {
            return false;
        }
        else if (key == "type" && type_ != value)
        {
            return false;
        }
        else if (key == "min_size" && size_ < std::stod(value))
        {
            return false;
        }
        else if (key == "max_size" && size_ > std::stod(value))
        {
            return false;
        }
    }

    return true;
}