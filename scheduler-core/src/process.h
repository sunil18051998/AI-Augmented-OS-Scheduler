#pragma once
#include <vector>
#include <string>


struct Process {
int pid;
double arrival_time;
double cpu_burst; // ground-truth (sim only)
double remaining;
int priority;
std::vector<double> features; // features fed to predictor
};