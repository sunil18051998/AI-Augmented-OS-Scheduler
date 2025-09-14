#include "process.h"

// Constructor
Process::Process(int pid_, double arrival, double burst_, int priority)
    : pid(pid_), arrival_time(arrival), burst_time(burst_), remaining(burst_),
      priority(priority), first_response_time(-1.0), finish_time(-1.0),
      features() // empty vector by default
{}

// Mark the first response time
void Process::markStart(double currentTime) {
    if (first_response_time < 0.0) {
        first_response_time = currentTime - arrival_time;
    }
}

// Mark process as finished
void Process::markFinish(double currentTime) {
    finish_time = currentTime;
}

// Update remaining burst time
void Process::updateRemaining(double time) {
    remaining -= time;
    if (remaining < 0.0) remaining = 0.0;
}
