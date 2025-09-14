#include "process.h"
#include <vector>
#include <fstream>
#include <sstream>

std::vector<Process> loadDatasetFromCSV(const std::string& path) {
    std::vector<Process> processes;
    std::ifstream file(path);
    if (!file.is_open()) throw std::runtime_error("Unable to open CSV file");

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        Process p{};
        char comma;
        ss >> p.pid >> comma >> p.arrivalTime >> comma >> p.burstTime >> comma >> p.priority;
        p.remainingTime = p.burstTime;
        processes.push_back(p);
    }
    return processes;
}
