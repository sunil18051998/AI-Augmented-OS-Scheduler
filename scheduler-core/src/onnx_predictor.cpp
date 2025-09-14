// #include "onnx_predictor.h"
// #include <iostream>
// #include <stdexcept>

// OnnxPredictor::OnnxPredictor(const std::string& modelPath)
//     : env(ORT_LOGGING_LEVEL_WARNING, "onnx_predictor")
// {
//     // Configure session options
//     Ort::SessionOptions session_options;
//     session_options.SetIntraOpNumThreads(1);
//     session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

//     // Convert modelPath to wide string (needed on Windows)
//     std::wstring wide_model_path(modelPath.begin(), modelPath.end());

//     // Load model
//     session = Ort::Session(env, wide_model_path.c_str(), session_options);
// }

// float OnnxPredictor::predictBurst(const std::vector<float>& features) {
//     std::cout << "[DEBUG] predictBurst() called, features size = " << features.size() << "\n";
//     if (features.empty()) {
//         throw std::invalid_argument("Input features cannot be empty");
//     }

//     // Create allocator and memory info
//     Ort::AllocatorWithDefaultOptions allocator;
//     Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
//         OrtArenaAllocator, OrtMemTypeDefault);

//     // Prepare input tensor shape: (1, feature_count)
//     std::vector<int64_t> input_shape = {1, static_cast<int64_t>(features.size())};

//     // Create input tensor
//     Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
//         memory_info,
//         const_cast<float*>(features.data()), // mutable pointer
//         features.size(),
//         input_shape.data(),
//         input_shape.size()
//     );

//     // Get input/output names
//     // char* input_name = session.GetInputNameAllocated(0, allocator);
//     // char* output_name = session.GetOutputNameAllocated(0, allocator);

//     // const char* input_names[] = { input_name };
//     // const char* output_names[] = { output_name };

//     Ort::AllocatedStringPtr input_name = session.GetInputNameAllocated(0, allocator);
// Ort::AllocatedStringPtr output_name = session.GetOutputNameAllocated(0, allocator);

// const char* input_names[] = { input_name.get() };
// const char* output_names[] = { output_name.get() };

//     // Run inference
//     auto output_tensors = session.Run(
//         Ort::RunOptions{nullptr},
//         input_names, &input_tensor, 1,
//         output_names, 1
//     );

//     // Get output tensor data
//     float* output_data = output_tensors.front().GetTensorMutableData<float>();
//     float result = output_data[0];

//     // Free allocated names
//     // allocator.Free(input_name);
//     // allocator.Free(output_name);

//     allocator.Free(input_name.get());
// allocator.Free(output_name.get());

//     return result;
// }



#include "onnx_predictor.h"
#include <iostream>
#include <locale>
#include <codecvt>

OnnxPredictor::OnnxPredictor(const std::string& modelPath)
    : env(ORT_LOGGING_LEVEL_WARNING, "onnx_predictor")
{
    // Convert std::string -> std::wstring (UTF-8 to UTF-16)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide_model_path = converter.from_bytes(modelPath);

    Ort::SessionOptions session_options;
    session = Ort::Session(env, wide_model_path.c_str(), session_options);

    std::cout << "[DEBUG] OnnxPredictor constructor: model loaded successfully\n";
}


float OnnxPredictor::predictBurst(const std::vector<float>& features) {
    std::cout << "[DEBUG] predictBurst() called, features size = " << features.size() << "\n";

    try {
        Ort::AllocatorWithDefaultOptions allocator;

        // --- Input Names ---
        size_t num_input_nodes = session.GetInputCount();
        std::cout << "[DEBUG] Num input nodes: " << num_input_nodes << "\n";

        Ort::AllocatedStringPtr input_name_alloc =
            session.GetInputNameAllocated(0, allocator);
        const char* input_name = input_name_alloc.get();
        std::cout << "[DEBUG] Input name: " << input_name << "\n";

        // --- Prepare Input Tensor ---
        std::vector<int64_t> input_shape = {1, static_cast<int64_t>(features.size())};
        Ort::MemoryInfo memory_info =
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            const_cast<float*>(features.data()), // non-const pointer required
            features.size(),
            input_shape.data(),
            input_shape.size()
        );

        // --- Output Names (RAII Safe) ---
        size_t num_outputs = session.GetOutputCount();
        std::vector<Ort::AllocatedStringPtr> output_name_allocs;
        std::vector<const char*> output_names;
        output_name_allocs.reserve(num_outputs);
        output_names.reserve(num_outputs);

        for (size_t i = 0; i < num_outputs; i++) {
            output_name_allocs.push_back(session.GetOutputNameAllocated(i, allocator));
            output_names.push_back(output_name_allocs.back().get());
        }

        // --- Run Inference ---
        auto output_tensors = session.Run(
            Ort::RunOptions{nullptr},
            &input_name,
            &input_tensor,
            1,
            output_names.data(),
            output_names.size()
        );

        std::cout << "[DEBUG] Inference completed, got "
                  << output_tensors.size() << " outputs\n";

        // --- Extract Result ---
        float* float_array = output_tensors.front().GetTensorMutableData<float>();
        float prediction = float_array[0];

        std::cout << "[DEBUG] Prediction value: " << prediction << "\n";
        return prediction;
    }
    catch (const Ort::Exception& e) {
        std::cerr << "[ERROR] ONNX Runtime Exception: " << e.what() << "\n";
        throw;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Standard Exception: " << e.what() << "\n";
        throw;
    }
}
