#ifndef API_SERVER_H
#define API_SERVER_H

#include "property_manager.h"
#include "search_engine.h"
#include "ml_predictor.h"
#include <string>
#include <map>

/**
 * APIServer class for handling HTTP requests (simplified implementation)
 */
class APIServer
{
public:
    APIServer(PropertyManager *pm, SearchEngine *se, MLPredictor *ml);
    ~APIServer();

    // Server control
    bool start(int port = 8080);
    void stop();
    bool isRunning() const { return is_running_; }

    // Request handlers
    std::string handleRequest(const std::string &path, const std::map<std::string, std::string> &params);

    // Specific endpoints
    std::string handleSearch(const std::map<std::string, std::string> &params);
    std::string handlePredict(const std::map<std::string, std::string> &params);
    std::string handleRecommend(const std::map<std::string, std::string> &params);
    std::string handleStats(const std::map<std::string, std::string> &params);
    std::string handleNearby(const std::map<std::string, std::string> &params);

    // Utility methods
    std::string propertiesToJSON(const std::vector<Property> &properties);
    std::string statsToJSON(const std::map<std::string, double> &stats);
    std::string errorResponse(const std::string &message);

    // Configuration
    void enableCORS(bool enable) { enable_cors_ = enable; }
    void setMaxResults(int max_results) { max_results_ = max_results; }

private:
    PropertyManager *property_manager_;
    SearchEngine *search_engine_;
    MLPredictor *ml_predictor_;

    bool is_running_;
    int port_;
    bool enable_cors_;
    int max_results_;

    // Helper methods
    std::map<std::string, std::string> parseQueryParams(const std::string &query);
    std::string urlDecode(const std::string &str);
    std::string escapeJSON(const std::string &str);
    std::string formatCORSHeaders();
};

#endif // API_SERVER_H