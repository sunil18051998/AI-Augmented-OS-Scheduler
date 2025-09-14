#pragma once

#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>

class OnnxPredictor {
public:
    explicit OnnxPredictor(const std::string& modelPath);
    float predictBurst(const std::vector<float>& features);

private:
    Ort::Env env;
    Ort::Session session{nullptr};  // Make sure this is assignable (not const)
};

