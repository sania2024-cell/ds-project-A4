#ifndef PROPERTY_H
#define PROPERTY_H

#include <string>
#include <vector>
#include <map>

/**
 * Property class representing a real estate property
 */
class Property
{
public:
    // Constructors
    Property();
    Property(int id, const std::string &city, double price, int bedrooms,
             int bathrooms, double size, const std::string &type,
             double latitude, double longitude);

    // Getters
    int getId() const { return id_; }
    std::string getCity() const { return city_; }
    double getPrice() const { return price_; }
    int getBedrooms() const { return bedrooms_; }
    int getBathrooms() const { return bathrooms_; }
    double getSize() const { return size_; }
    std::string getType() const { return type_; }
    double getLatitude() const { return latitude_; }
    double getLongitude() const { return longitude_; }
    std::vector<std::string> getAmenities() const { return amenities_; }
    double getPredictedPrice() const { return predicted_price_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setCity(const std::string &city) { city_ = city; }
    void setPrice(double price) { price_ = price; }
    void setBedrooms(int bedrooms) { bedrooms_ = bedrooms; }
    void setBathrooms(int bathrooms) { bathrooms_ = bathrooms; }
    void setSize(double size) { size_ = size; }
    void setType(const std::string &type) { type_ = type; }
    void setLatitude(double latitude) { latitude_ = latitude; }
    void setLongitude(double longitude) { longitude_ = longitude; }
    void setAmenities(const std::vector<std::string> &amenities) { amenities_ = amenities; }
    void setPredictedPrice(double predicted_price) { predicted_price_ = predicted_price; }

    // Utility methods
    std::string toString() const;
    std::map<std::string, std::string> toMap() const;
    bool matchesFilter(const std::map<std::string, std::string> &filters) const;

private:
    int id_;
    std::string city_;
    double price_;
    int bedrooms_;
    int bathrooms_;
    double size_;
    std::string type_;
    double latitude_;
    double longitude_;
    std::vector<std::string> amenities_;
    double predicted_price_;
};

#endif // PROPERTY_H