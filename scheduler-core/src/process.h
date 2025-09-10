#pragma once
#include <vector>

struct Process {
    int pid;
    double arrival_time;       // arrival time of the process
    double burst;              // total burst time
    double remaining;          // remaining burst time
    int initial_priority;      // priority for MLFQ or other algorithms

    double first_response_time; // first response time
    double finish_time;         // finish time

    std::vector<double> features; // feature vector for predictor

    // Constructor
    Process(int pid_, double arrival, double burst_, int priority);

    // Methods
    void markStart(double currentTime);
    void markFinish(double currentTime);
    void updateRemaining(double time);
};
