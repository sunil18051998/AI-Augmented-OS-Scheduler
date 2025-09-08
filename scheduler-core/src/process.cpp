// process.cpp
#include "process.h"

Process::Process(int pid, int arrival, int burst, int priority)
    : pid(pid),
      arrivalTime(arrival),
      burstTime(burst),
      remainingTime(burst),
      priority(priority),
      startTime(-1),
      finishTime(-1),
      waitingTime(0),
      turnaroundTime(0) {}

void Process::updateWaitingTime(int currentTime) {
    waitingTime = currentTime - arrivalTime - (burstTime - remainingTime);
}

void Process::markStart(int currentTime) {
    if (startTime == -1) {
        startTime = currentTime;
    }
}

void Process::markFinish(int currentTime) {
    finishTime = currentTime;
    turnaroundTime = finishTime - arrivalTime;
    waitingTime = turnaroundTime - burstTime;
}
