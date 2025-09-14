#include <iostream>
#include <vector>
#include <string>
#include "scheduler.h"
#include "onnx_predictor.h"
#include "process.h"
#include "csv_loader.h"  // helper for reading dataset from CSV

int main(int argc, char** argv) {
    std::cout << "[DEBUG] Program started\n";

    // Load dataset
    std::string datasetPath = "../../datasets/sample_processes.csv";
    std::vector<Process> processes = loadProcessesFromCSV(datasetPath);
    std::cout << "[DEBUG] Loaded " << processes.size() << " processes from CSV\n";

    // Initialize ML predictor
    OnnxPredictor predictor("../../ml-predictor/burst_model.onnx");

    // Predict burst times for processes missing it
    for (auto& p : processes) {
        if (p.burst_time <= 0) {
            std::vector<float> features = { (float)p.arrival_time, (float)p.priority, (float)p.io_time, (float)p.memory_req };
            p.burst_time = predictor.predictBurst(features);
            std::cout << "[DEBUG] Predicted burst for PID " << p.pid << ": " << p.burst_time << "\n";
        }
    }

    // Select algorithm (can be passed as arg from Node.js)
    std::string algo = "RR";  // default
    if (argc > 1) algo = argv[1];

    std::cout << "[DEBUG] Running algorithm: " << algo << "\n";

    Scheduler scheduler(algo);
    SchedulerResult result = scheduler.run(processes);

    std::cout << "\n=== Simulation Result ===\n";
    std::cout << "Average Waiting Time: " << result.avg_waiting_time << "\n";
    std::cout << "Average Turnaround Time: " << result.avg_turnaround_time << "\n";
    std::cout << "CPU Utilization: " << result.cpu_utilization << "%\n";
    std::cout << "Throughput: " << result.throughput << " processes/unit time\n";
    std::cout << "Context Switches: " << result.context_switches << "\n";

    std::cout << "[DEBUG] Program exiting normally\n";
    return 0;
}
