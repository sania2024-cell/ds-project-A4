#include "ml_predictor.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <fstream>

MLPredictor::MLPredictor() : model_type_("linear_regression"), is_trained_(false), bias_(0.0) {}

MLPredictor::~MLPredictor() {}

bool MLPredictor::trainModel(const std::vector<Property> &training_data)
{
    if (training_data.empty())
    {
        std::cerr << "Error: No training data provided" << std::endl;
        return false;
    }

    std::cout << "Training ML model with " << training_data.size() << " properties..." << std::endl;

    // Initialize encodings for categorical variables
    initializeEncodings(training_data);

    // Extract features and targets
    std::vector<std::vector<double>> X;
    std::vector<double> y;

    for (const auto &property : training_data)
    {
        X.push_back(extractFeatures(property));
        y.push_back(property.getPrice());
    }

    // Calculate normalization parameters
    calculateNormalizationParams(X);

    // Normalize features
    for (auto &features : X)
    {
        features = normalizeFeatureVector(features);
    }

    // Train the model
    if (model_type_ == "linear_regression")
    {
        trainLinearRegression(X, y);
    }

    is_trained_ = true;
    std::cout << "Model training completed!" << std::endl;
    return true;
}

double MLPredictor::predictPrice(const Property &property)
{
    if (!is_trained_)
    {
        std::cerr << "Error: Model not trained yet" << std::endl;
        return 0.0;
    }

    auto features = extractFeatures(property);
    auto normalized_features = normalizeFeatureVector(features);

    return predict(normalized_features);
}

std::vector<double> MLPredictor::predictPrices(const std::vector<Property> &properties)
{
    std::vector<double> predictions;

    for (const auto &property : properties)
    {
        predictions.push_back(predictPrice(property));
    }

    return predictions;
}

std::vector<double> MLPredictor::extractFeatures(const Property &property)
{
    std::vector<double> features;

    // Categorical features (encoded)
    features.push_back(encodeCategorical(property.getCity(), city_encoding_));
    features.push_back(encodeCategorical(property.getType(), type_encoding_));

    // Numerical features
    features.push_back(static_cast<double>(property.getBedrooms()));
    features.push_back(static_cast<double>(property.getBathrooms()));
    features.push_back(property.getSize());
    features.push_back(property.getLatitude());
    features.push_back(property.getLongitude());

    // Derived features
    features.push_back(property.getSize() / property.getBedrooms());         // Size per bedroom
    features.push_back(static_cast<double>(property.getAmenities().size())); // Amenity count

    return features;
}

void MLPredictor::normalizeFeatures(std::vector<double> &features)
{
    features = normalizeFeatureVector(features);
}

double MLPredictor::calculateAccuracy(const std::vector<Property> &test_data)
{
    if (!is_trained_ || test_data.empty())
    {
        return 0.0;
    }

    double total_error = 0.0;
    double total_actual = 0.0;

    for (const auto &property : test_data)
    {
        double predicted = predictPrice(property);
        double actual = property.getPrice();

        total_error += std::abs(predicted - actual);
        total_actual += actual;
    }

    // Mean Absolute Percentage Error (MAPE)
    double mape = (total_error / total_actual) * 100.0;
    return 100.0 - mape; // Convert to accuracy percentage
}

std::map<std::string, double> MLPredictor::getModelMetrics()
{
    std::map<std::string, double> metrics;

    metrics["is_trained"] = is_trained_ ? 1.0 : 0.0;
    metrics["num_features"] = static_cast<double>(weights_.size());
    metrics["bias"] = bias_;

    if (!weights_.empty())
    {
        double weight_sum = std::accumulate(weights_.begin(), weights_.end(), 0.0);
        metrics["avg_weight"] = weight_sum / weights_.size();
    }

    return metrics;
}

bool MLPredictor::loadModel(const std::string &model_file)
{
    std::ifstream file(model_file);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open model file " << model_file << std::endl;
        return false;
    }

    // Simple text format for model loading
    std::string line;

    // Read model type
    if (std::getline(file, line))
    {
        model_type_ = line;
    }

    // Read bias
    if (std::getline(file, line))
    {
        bias_ = std::stod(line);
    }

    // Read weights
    weights_.clear();
    while (std::getline(file, line) && !line.empty())
    {
        weights_.push_back(std::stod(line));
    }

    file.close();
    is_trained_ = !weights_.empty();

    std::cout << "Model loaded from " << model_file << std::endl;
    return true;
}

bool MLPredictor::saveModel(const std::string &model_file)
{
    if (!is_trained_)
    {
        std::cerr << "Error: Cannot save untrained model" << std::endl;
        return false;
    }

    std::ofstream file(model_file);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot create model file " << model_file << std::endl;
        return false;
    }

    // Save model type
    file << model_type_ << std::endl;

    // Save bias
    file << bias_ << std::endl;

    // Save weights
    for (double weight : weights_)
    {
        file << weight << std::endl;
    }

    file.close();
    std::cout << "Model saved to " << model_file << std::endl;
    return true;
}

void MLPredictor::initializeEncodings(const std::vector<Property> &data)
{
    city_encoding_.clear();
    type_encoding_.clear();

    for (const auto &property : data)
    {
        encodeCategorical(property.getCity(), city_encoding_);
        encodeCategorical(property.getType(), type_encoding_);
    }
}

int MLPredictor::encodeCategorical(const std::string &value, std::map<std::string, int> &encoding)
{
    auto it = encoding.find(value);
    if (it != encoding.end())
    {
        return it->second;
    }
    else
    {
        int new_code = static_cast<int>(encoding.size());
        encoding[value] = new_code;
        return new_code;
    }
}

void MLPredictor::calculateNormalizationParams(const std::vector<std::vector<double>> &features)
{
    if (features.empty())
        return;

    size_t num_features = features[0].size();
    feature_means_.resize(num_features, 0.0);
    feature_stds_.resize(num_features, 0.0);

    // Calculate means
    for (const auto &feature_vec : features)
    {
        for (size_t i = 0; i < num_features; ++i)
        {
            feature_means_[i] += feature_vec[i];
        }
    }

    for (size_t i = 0; i < num_features; ++i)
    {
        feature_means_[i] /= features.size();
    }

    // Calculate standard deviations
    for (const auto &feature_vec : features)
    {
        for (size_t i = 0; i < num_features; ++i)
        {
            feature_stds_[i] += std::pow(feature_vec[i] - feature_means_[i], 2);
        }
    }

    for (size_t i = 0; i < num_features; ++i)
    {
        feature_stds_[i] = std::sqrt(feature_stds_[i] / features.size());
        if (feature_stds_[i] == 0.0)
            feature_stds_[i] = 1.0; // Avoid division by zero
    }
}

std::vector<double> MLPredictor::normalizeFeatureVector(const std::vector<double> &features)
{
    std::vector<double> normalized = features;

    for (size_t i = 0; i < normalized.size() && i < feature_means_.size(); ++i)
    {
        normalized[i] = (normalized[i] - feature_means_[i]) / feature_stds_[i];
    }

    return normalized;
}

void MLPredictor::trainLinearRegression(const std::vector<std::vector<double>> &X, const std::vector<double> &y)
{
    if (X.empty() || X.size() != y.size())
    {
        std::cerr << "Error: Invalid training data dimensions" << std::endl;
        return;
    }

    size_t num_features = X[0].size();
    size_t num_samples = X.size();

    weights_.resize(num_features, 0.0);
    bias_ = 0.0;

    // Simple gradient descent implementation
    double learning_rate = 0.01;
    int max_iterations = 1000;

    for (int iter = 0; iter < max_iterations; ++iter)
    {
        std::vector<double> weight_gradients(num_features, 0.0);
        double bias_gradient = 0.0;

        // Calculate gradients
        for (size_t i = 0; i < num_samples; ++i)
        {
            double predicted = predict(X[i]);
            double error = predicted - y[i];

            for (size_t j = 0; j < num_features; ++j)
            {
                weight_gradients[j] += error * X[i][j];
            }
            bias_gradient += error;
        }

        // Update weights
        for (size_t j = 0; j < num_features; ++j)
        {
            weights_[j] -= learning_rate * weight_gradients[j] / num_samples;
        }
        bias_ -= learning_rate * bias_gradient / num_samples;

        // Print progress every 100 iterations
        if (iter % 100 == 0)
        {
            double mse = 0.0;
            for (size_t i = 0; i < num_samples; ++i)
            {
                double error = predict(X[i]) - y[i];
                mse += error * error;
            }
            mse /= num_samples;
            std::cout << "Iteration " << iter << ", MSE: " << mse << std::endl;
        }
    }
}

double MLPredictor::predict(const std::vector<double> &features)
{
    if (features.size() != weights_.size())
    {
        std::cerr << "Error: Feature dimension mismatch" << std::endl;
        return 0.0;
    }

    double result = bias_;
    for (size_t i = 0; i < features.size(); ++i)
    {
        result += weights_[i] * features[i];
    }

    return std::max(0.0, result); // Ensure non-negative price
}