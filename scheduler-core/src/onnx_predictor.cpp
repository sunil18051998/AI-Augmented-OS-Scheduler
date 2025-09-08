// onnx_predictor.cpp
#include "onnx_predictor.h"
#include <iostream>

ONNXPredictor::ONNXPredictor(const std::string& modelPath) {
    env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "scheduler");
    sessionOptions = std::make_unique<Ort::SessionOptions>();
    sessionOptions->SetIntraOpNumThreads(1);
    sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    session = std::make_unique<Ort::Session>(*env, modelPath.c_str(), *sessionOptions);

    allocator = std::make_unique<Ort::AllocatorWithDefaultOptions>();
    inputName = session->GetInputNameAllocated(0, *allocator).get();
    outputName = session->GetOutputNameAllocated(0, *allocator).get();
}

float ONNXPredictor::predictBurst(const std::vector<float>& features) {
    Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(
        OrtDeviceAllocator, OrtMemTypeCPU);

    // Create input tensor
    std::array<int64_t, 2> inputShape{1, (int64_t)features.size()};
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memInfo, const_cast<float*>(features.data()), features.size(),
        inputShape.data(), inputShape.size());

    // Run model
    auto outputTensors = session->Run(
        Ort::RunOptions{nullptr},
        &inputName, &inputTensor, 1,
        &outputName, 1);

    // Extract result
    float* outputArr = outputTensors.front().GetTensorMutableData<float>();
    return outputArr[0];
}
