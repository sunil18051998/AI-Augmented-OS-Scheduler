#pragma once
#include "process.h"
#include "predictor.h"
#include <vector>
#include <memory>
#include <string>

struct ScheduleEvent {
    double time;
    int pid;
    double duration;
    std::string queue; // e.g., "RR", "Q0", "ML_RR"
};

struct Metrics {
    double avg_turnaround = 0.0;
    double avg_waiting = 0.0;
    double avg_response = 0.0;
    double cpu_utilization = 0.0;
    int processed = 0;
};

enum class Algo {
    RR,
    MLFQ,
    ML_RR
};

class Scheduler {
public:
    Scheduler(std::shared_ptr<PredictorInterface> predictor);
    // run simulation, events pushed into vector (in chronological order)
    Metrics run(const std::vector<Process>& processes, Algo algo, double quantum, double mlfq_base_quantum, bool verbose, std::vector<ScheduleEvent>& events_out);

private:
    std::shared_ptr<PredictorInterface> predictor;

    Metrics compute_metrics(const std::vector<Process>& procs, double total_time, double cpu_busy_time);
};
