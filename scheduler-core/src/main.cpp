#include "onnx_predictor.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "[DEBUG] Program started\n";

    try {
        std::string model_path = "../../ml-predictor/burst_model.onnx";
        std::cout << "[DEBUG] Model path: " << model_path << "\n";

        OnnxPredictor predictor(model_path);
        std::cout << "[DEBUG] Predictor initialized\n";

        std::vector<float> features = {100.0f, 2.0f, 15.0f, 7.0f};
        std::cout << "[DEBUG] Features ready\n";

        float prediction = predictor.predictBurst(features);
        std::cout << "[DEBUG] Prediction done\n";

        std::cout << "Predicted burst time: " << prediction << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[DEBUG] Program exiting normally\n";
    return 0;
}
