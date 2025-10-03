#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <chrono>

/**
 * Utility functions for the Property Finder application
 */
namespace Utils
{

    // String utilities
    std::string trim(const std::string &str);
    std::string toLowerCase(const std::string &str);
    std::string toUpperCase(const std::string &str);
    std::vector<std::string> split(const std::string &str, char delimiter);
    std::string join(const std::vector<std::string> &strings, const std::string &delimiter);

    // File utilities
    bool fileExists(const std::string &filename);
    std::string getFileExtension(const std::string &filename);
    std::string getFileName(const std::string &filepath);

    // Time utilities
    std::string getCurrentTimestamp();
    std::string formatDuration(std::chrono::milliseconds duration);

    // Math utilities
    double roundToDecimalPlaces(double value, int decimal_places);
    double calculatePercentage(double part, double whole);

    // Validation utilities
    bool isValidEmail(const std::string &email);
    bool isValidPhoneNumber(const std::string &phone);
    bool isValidCoordinate(double latitude, double longitude);

    // Logging utilities
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    void log(LogLevel level, const std::string &message);
    void setLogLevel(LogLevel level);

    // Performance measurement
    class Timer
    {
    public:
        Timer();
        void start();
        void stop();
        std::chrono::milliseconds elapsed() const;
        void reset();

    private:
        std::chrono::steady_clock::time_point start_time_;
        std::chrono::steady_clock::time_point end_time_;
        bool is_running_;
    };
}

#endif // UTILS_H