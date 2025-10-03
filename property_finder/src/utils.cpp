#include "utils.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <iomanip>
#include <cmath>

namespace Utils
{

    std::string trim(const std::string &str)
    {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            return "";

        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    std::string toLowerCase(const std::string &str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    std::string toUpperCase(const std::string &str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    std::vector<std::string> split(const std::string &str, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter))
        {
            tokens.push_back(token);
        }

        return tokens;
    }

    std::string join(const std::vector<std::string> &strings, const std::string &delimiter)
    {
        if (strings.empty())
            return "";

        std::ostringstream oss;
        for (size_t i = 0; i < strings.size(); ++i)
        {
            oss << strings[i];
            if (i < strings.size() - 1)
            {
                oss << delimiter;
            }
        }

        return oss.str();
    }

    bool fileExists(const std::string &filename)
    {
        std::ifstream file(filename);
        return file.good();
    }

    std::string getFileExtension(const std::string &filename)
    {
        size_t dot_pos = filename.find_last_of('.');
        if (dot_pos != std::string::npos && dot_pos != filename.length() - 1)
        {
            return filename.substr(dot_pos + 1);
        }
        return "";
    }

    std::string getFileName(const std::string &filepath)
    {
        size_t slash_pos = filepath.find_last_of("/\\");
        if (slash_pos != std::string::npos)
        {
            return filepath.substr(slash_pos + 1);
        }
        return filepath;
    }

    std::string getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string formatDuration(std::chrono::milliseconds duration)
    {
        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1));
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration % std::chrono::minutes(1));
        auto ms = duration % std::chrono::seconds(1);

        std::ostringstream oss;

        if (hours.count() > 0)
        {
            oss << hours.count() << "h ";
        }
        if (minutes.count() > 0)
        {
            oss << minutes.count() << "m ";
        }
        if (seconds.count() > 0 || hours.count() == 0)
        {
            oss << seconds.count();
            if (ms.count() > 0 && hours.count() == 0 && minutes.count() == 0)
            {
                oss << "." << std::setfill('0') << std::setw(3) << ms.count();
            }
            oss << "s";
        }

        return oss.str();
    }

    double roundToDecimalPlaces(double value, int decimal_places)
    {
        double multiplier = std::pow(10.0, decimal_places);
        return std::round(value * multiplier) / multiplier;
    }

    double calculatePercentage(double part, double whole)
    {
        if (whole == 0.0)
            return 0.0;
        return (part / whole) * 100.0;
    }

    bool isValidEmail(const std::string &email)
    {
        const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(email, pattern);
    }

    bool isValidPhoneNumber(const std::string &phone)
    {
        const std::regex pattern(R"(^\+?[1-9]\d{1,14}$)");
        std::string cleaned_phone = phone;
        // Remove spaces and hyphens
        cleaned_phone.erase(std::remove_if(cleaned_phone.begin(), cleaned_phone.end(),
                                           [](char c)
                                           { return c == ' ' || c == '-' || c == '(' || c == ')'; }),
                            cleaned_phone.end());

        return std::regex_match(cleaned_phone, pattern);
    }

    bool isValidCoordinate(double latitude, double longitude)
    {
        return (latitude >= -90.0 && latitude <= 90.0) &&
               (longitude >= -180.0 && longitude <= 180.0);
    }

    // Global log level
    static LogLevel current_log_level = LogLevel::INFO;

    void log(LogLevel level, const std::string &message)
    {
        if (level < current_log_level)
            return;

        std::string level_str;
        switch (level)
        {
        case LogLevel::DEBUG:
            level_str = "[DEBUG]";
            break;
        case LogLevel::INFO:
            level_str = "[INFO]";
            break;
        case LogLevel::WARNING:
            level_str = "[WARN]";
            break;
        case LogLevel::ERROR:
            level_str = "[ERROR]";
            break;
        }

        std::cout << getCurrentTimestamp() << " " << level_str << " " << message << std::endl;
    }

    void setLogLevel(LogLevel level)
    {
        current_log_level = level;
    }

    // Timer implementation
    Timer::Timer() : is_running_(false) {}

    void Timer::start()
    {
        start_time_ = std::chrono::steady_clock::now();
        is_running_ = true;
    }

    void Timer::stop()
    {
        if (is_running_)
        {
            end_time_ = std::chrono::steady_clock::now();
            is_running_ = false;
        }
    }

    std::chrono::milliseconds Timer::elapsed() const
    {
        if (is_running_)
        {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
        }
        else
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(end_time_ - start_time_);
        }
    }

    void Timer::reset()
    {
        is_running_ = false;
        start_time_ = std::chrono::steady_clock::time_point{};
        end_time_ = std::chrono::steady_clock::time_point{};
    }
}