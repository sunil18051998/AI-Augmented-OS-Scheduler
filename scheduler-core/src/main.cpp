

#include "onnx_predictor.h"
#include <iostream>
#include <vector>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>

int main() {
    try {
        //std::cerr << "[DEBUG] Program started\n";

        // Example: Path to ONNX model
        std::string model_path = "../../ml-predictor/burst_model.onnx";
        OnnxPredictor predictor(model_path);

        //std::cerr << "[DEBUG] Predictor initialized\n";

        // Example input features (could come from config or process list)
        std::vector<std::vector<float>> process_features = {
            {100.0f, 2.0f, 15.0f, 7.0f},
            {120.0f, 1.0f, 12.0f, 6.0f},
            {140.0f, 3.0f, 20.0f, 5.0f}
        };

        // Create JSON result
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        writer.StartArray();

        float current_time = 0.0f;

        for (size_t i = 0; i < process_features.size(); ++i) {
            float prediction = predictor.predictBurst(process_features[i]);

            writer.StartObject();
            writer.Key("pid");
            writer.String(("P" + std::to_string(i + 1)).c_str());

            writer.Key("time");
            writer.Double(current_time);

            writer.Key("duration");
            writer.Double(prediction);
            writer.EndObject();

            current_time += prediction;
        }

        writer.EndArray();

        // Print JSON to stdout (so Node backend can read it)
        std::cout << buffer.GetString() << std::endl;

        //std::cerr << "[DEBUG] Program exiting normally\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Fatal: " << e.what() << std::endl;
        return 1;
    }
}
