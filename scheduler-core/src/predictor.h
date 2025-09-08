#pragma once
#include <vector>
#include <random>

// Abstract predictor interface
class PredictorInterface {
public:
    virtual ~PredictorInterface() = default;
    // Return predicted remaining burst in same time units as scheduler (double)
    virtual double predict(const std::vector<double>& features, double fallback) = 0;
};

// Dummy predictor: returns either fallback (ground-truth remaining) or a noisy estimate.
// Replace this with an ONNX Runtime wrapper without changing scheduler code.
class DummyPredictor : public PredictorInterface {
public:
    DummyPredictor(double noise_std = 0.0) : gen(std::random_device{}()), dist(0.0, noise_std) {}

    double predict(const std::vector<double>& /*features*/, double fallback) override {
        double noise = dist(gen);
        double pred = fallback + noise;
        if (pred < 0.0) pred = 0.0;
        return pred;
    }
private:
    std::mt19937 gen;
    std::normal_distribution<double> dist;
};
