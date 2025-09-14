#pragma once
#include <vector>
#include <string>
#include "process.h"

struct GanttEvent {
    int pid;
    int start_time;
    int duration;
};

struct SchedulerResult {
    double avg_waiting_time;
    double avg_turnaround_time;
    double cpu_utilization;
    double throughput;
    int context_switches;
    std::vector<GanttEvent> gantt_chart;  // timeline for frontend
};

class Scheduler {
public:
    Scheduler(std::string algorithm, int time_quantum = 4);
    SchedulerResult run(std::vector<Process>& processes);

private:
    std::string algo;
    int time_quantum;

    SchedulerResult runFCFS(std::vector<Process>& processes);
    SchedulerResult runSJF(std::vector<Process>& processes);
    SchedulerResult runRR(std::vector<Process>& processes);
    SchedulerResult runMLFQ(std::vector<Process>& processes);
};
