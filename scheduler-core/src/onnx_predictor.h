// onnx_predictor.h
#pragma once
#include <onnxruntime_cxx_api.h>
#include <memory>
#include <string>
#include <vector>

class ONNXPredictor {
public:
    explicit ONNXPredictor(const std::string& modelPath);
    float predictBurst(const std::vector<float>& features);

private:
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::SessionOptions> sessionOptions;
    std::unique_ptr<Ort::Session> session;
    std::unique_ptr<Ort::AllocatorWithDefaultOptions> allocator;

    std::string inputName;
    std::string outputName;
};
