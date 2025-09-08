// process.h
#pragma once
#include <string>

struct Process {
    int pid;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int priority;

    int startTime;
    int finishTime;
    int waitingTime;
    int turnaroundTime;

    Process(int pid, int arrival, int burst, int priority);

    void updateWaitingTime(int currentTime);
    void markStart(int currentTime);
    void markFinish(int currentTime);
};
