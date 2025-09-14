#pragma once
#include <vector>
#include <string>
#include "process.h"

// Loads processes from a CSV file.
// Expected columns (with header):
// pid,arrival_time,burst_time,priority,io_time,memory_req
std::vector<Process> loadProcessesFromCSV(const std::string& filename);
