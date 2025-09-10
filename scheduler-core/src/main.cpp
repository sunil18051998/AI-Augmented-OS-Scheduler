
#include "onnx_predictor.h"
#include <iostream>
#include <vector>

int main() {
    try {
        // Path to your exported ONNX model (adjust if needed)
        std::string model_path = "../ml-predictor/burst_model.onnx";

        // Initialize predictor
        OnnxPredictor predictor(model_path);

        // Example features: [arrival, priority, io_wait, prev_burst]
        std::vector<float> features = {100.0f, 2.0f, 15.0f, 7.0f};

        // Run inference
        float prediction = predictor.predictBurst(features);

        std::cout << "Predicted burst time: " << prediction << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
