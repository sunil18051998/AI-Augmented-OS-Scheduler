#include "csv_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<Process> loadProcessesFromCSV(const std::string& filename) {
    std::vector<Process> processes;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "[ERROR] Could not open CSV file: " << filename << "\n";
        return processes;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        Process p(0, 0.0, 0.0, 0);
        //Process p(0, 0.0, 0.0, 0, 0.0, 0.0);

        if (isHeader) {
            // skip header line
            isHeader = false;
            continue;
        }

        // CSV format: pid,arrival_time,burst_time,priority,io_time,memory_req
        try {
            std::getline(ss, token, ','); p.pid = std::stoi(token);
            std::getline(ss, token, ','); p.arrival_time = std::stod(token);
            std::getline(ss, token, ','); p.burst_time = std::stod(token);  // may be 0 -> predictor will handle it
            std::getline(ss, token, ','); p.priority = std::stoi(token);
            std::getline(ss, token, ','); p.io_time = std::stod(token);
            std::getline(ss, token, ','); p.memory_req = std::stod(token);
        } catch (...) {
            std::cerr << "[WARNING] Skipping invalid line: " << line << "\n";
            continue;
        }

        processes.push_back(p);
    }

    std::cout << "[DEBUG] Loaded " << processes.size() << " valid process entries from CSV\n";
    return processes;
}
