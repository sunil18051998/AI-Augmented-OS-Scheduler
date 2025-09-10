#pragma once

#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>

class OnnxPredictor {
public:
    // Constructor loads the model
    explicit OnnxPredictor(const std::string& modelPath);

    // Run inference: features -> burst prediction
    float predictBurst(const std::vector<float>& features);

private:
    Ort::Env env;       // ONNX Runtime environment
    Ort::Session session{nullptr};  // The loaded ONNX model
};
