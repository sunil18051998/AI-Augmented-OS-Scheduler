#pragma once
#include <vector>
#include "process.h"

struct SimulationResult {
    std::vector<Process> gantt; // Ordered execution timeline
    double avgWaitingTime;
    double avgTurnaroundTime;
    double cpuUtilization;
    double throughput;
    int contextSwitches;
};
