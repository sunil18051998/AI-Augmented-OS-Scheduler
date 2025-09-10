#include <iostream>
#include <vector>
#include <onnxruntime_cxx_api.h>

int main() {
    // Initialize ONNX Runtime environment
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "onnx_demo");

    // Create default allocator
    Ort::AllocatorWithDefaultOptions allocator;

    // Example input data
    std::vector<float> input_data = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<int64_t> input_shape = {2, 2}; // 2x2 tensor

    // Create CPU memory info
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator, OrtMemTypeDefault
    );

    // Create input tensor
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,            // OrtMemoryInfo*
        input_data.data(),      // pointer to your data
        input_data.size(),      // number of elements
        input_shape.data(),     // tensor shape
        input_shape.size()      // number of dimensions
    );

    // Just a test print to confirm shape
    std::cout << "Tensor created with shape: ";
    for (auto dim : input_shape) {
        std::cout << dim << " ";
    }
    std::cout << std::endl;

    return 0;
}
