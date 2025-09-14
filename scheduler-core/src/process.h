#pragma once
#include <vector>

struct Process {
    int pid;
    double arrival_time;       // arrival time of the process
    int burst_time;              // total burst time
    double remaining;          // remaining burst time
    int priority;      // priority for MLFQ or other algorithms

    double first_response_time; // first response time
    double finish_time;         // finish time
    double io_time;             // I/O time (not used in basic algorithms)
    double memory_req;         // memory requirement (not used in basic algorithms)
    double waiting_time;       // waiting time
    double turnaround_time;    // turnaround time
    double start_time;         // start time for Gantt chart
    double completion_time;           // end time for Gantt chart


    std::vector<double> features; // feature vector for predictor

    // Constructor
    Process(int pid_, double arrival, double burst_, int priority);

    // Methods
    void markStart(double currentTime);
    void markFinish(double currentTime);
    void updateRemaining(double time);
};
