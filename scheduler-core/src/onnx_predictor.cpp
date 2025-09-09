#include "onnx_predictor.h"
#include <onnxruntime_cxx_api.h>
#include <iostream>

ONNXPredictor::ONNXPredictor(const std::string& model_path) {
    try {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "BurstPredictor");
        session_options = std::make_unique<Ort::SessionOptions>();
        session_options->SetIntraOpNumThreads(1);
        session_options->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        session = std::make_unique<Ort::Session>(*env, model_path.c_str(), *session_options);

        allocator = std::make_unique<Ort::AllocatorWithDefaultOptions>();
        input_name = session->GetInputName(0, *allocator);
        output_name = session->GetOutputName(0, *allocator);
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime init error: " << e.what() << std::endl;
        throw;
    }
}

float ONNXPredictor::predictBurst(const std::vector<float>& features) {
    try {
        if (features.size() != 4) {
            throw std::runtime_error("Expected exactly 4 features: [arrival, priority, io_wait, prev_burst]");
        }

        // Shape: 1x4
        std::array<int64_t, 2> input_shape{1, 4};
        Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        // Copy features into float buffer
        std::vector<float> input_tensor_values(features);

        // Create tensor
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            mem_info, input_tensor_values.data(), input_tensor_values.size(),
            input_shape.data(), input_shape.size()
        );

        // Run inference
        auto output_tensors = session->Run(
            Ort::RunOptions{nullptr},
            &input_name, &input_tensor, 1,
            &output_name, 1
        );

        float* output_data = output_tensors.front().GetTensorMutableData<float>();
        return output_data[0];
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime inference error: " << e.what() << std::endl;
        return -1.0f;
    }
}
