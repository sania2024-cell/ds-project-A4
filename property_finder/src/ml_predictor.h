#ifndef ML_PREDICTOR_H
#define ML_PREDICTOR_H

#include "property.h"
#include <vector>
#include <map>

/**
 * MLPredictor class for machine learning-based price prediction and recommendations
 */
class MLPredictor
{
public:
    MLPredictor();
    ~MLPredictor();

    // Training methods
    bool trainModel(const std::vector<Property> &training_data);
    bool loadModel(const std::string &model_file);
    bool saveModel(const std::string &model_file);

    // Prediction methods
    double predictPrice(const Property &property);
    std::vector<double> predictPrices(const std::vector<Property> &properties);

    // Feature engineering
    std::vector<double> extractFeatures(const Property &property);
    void normalizeFeatures(std::vector<double> &features);

    // Model evaluation
    double calculateAccuracy(const std::vector<Property> &test_data);
    std::map<std::string, double> getModelMetrics();

    // Configuration
    void setModelType(const std::string &type) { model_type_ = type; }
    std::string getModelType() const { return model_type_; }

private:
    std::string model_type_;
    bool is_trained_;

    // Simple linear regression coefficients
    std::vector<double> weights_;
    double bias_;

    // Feature normalization parameters
    std::vector<double> feature_means_;
    std::vector<double> feature_stds_;

    // City and type encodings
    std::map<std::string, int> city_encoding_;
    std::map<std::string, int> type_encoding_;

    // Helper methods
    void initializeEncodings(const std::vector<Property> &data);
    int encodeCategorical(const std::string &value, std::map<std::string, int> &encoding);
    void calculateNormalizationParams(const std::vector<std::vector<double>> &features);
    std::vector<double> normalizeFeatureVector(const std::vector<double> &features);

    // Simple ML implementations
    void trainLinearRegression(const std::vector<std::vector<double>> &X, const std::vector<double> &y);
    double predict(const std::vector<double> &features);
};

#endif // ML_PREDICTOR_H