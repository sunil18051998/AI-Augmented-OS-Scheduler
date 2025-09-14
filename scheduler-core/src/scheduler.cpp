// // scheduler.cpp
// #include "scheduler.h"
// #include <queue>
// #include <deque>
// #include <algorithm>
// #include <iostream>
// #include <cmath>
// #include <fstream>
// #include <sstream>
// #include <vector>
// #include <limits>

// struct Metrics {
//     double turnaround_time;
//     double waiting_time;
//     double response_time;
//     int context_switches;
//     double cpu_utilization;
//     double total_time;
// };

// Scheduler::Scheduler(std::shared_ptr<PredictorInterface> predictor) : predictor(predictor) {}

// static bool all_done(const std::vector<Process>& procs) {
//     for (const auto& p : procs) if (p.remaining > 1e-9) return false;
//     return true;
// }

// // compute metrics helper
// Metrics Scheduler::compute_metrics(const std::vector<Process>& procs, double total_time, double cpu_busy_time, int context_switches) {
//     Metrics m;
//     double sum_tat = 0.0, sum_wait = 0.0, sum_resp = 0.0;
//     int count = 0;
//     for (const auto& p : procs) {
//         if (p.finish_time < 0.0) continue;
//         double tat = p.finish_time - p.arrival_time;
//         double wait = tat - p.burst_time;
//         double resp = p.first_response_time < 0.0 ? 0.0 : p.first_response_time;
//         sum_tat += tat;
//         sum_wait += wait;
//         sum_resp += resp;
//         ++count;
//     }
//     if (count > 0) {
//         m.processed = count;
//         m.avg_turnaround = sum_tat / count;
//         m.avg_waiting = sum_wait / count;
//         m.avg_response = sum_resp / count;
//     }
//     m.cpu_utilization = total_time > 0.0 ? (cpu_busy_time / total_time) * 100.0 : 0.0;
//     m.context_switches = context_switches;
//     m.total_time = total_time;
//     return m;
// }

// Metrics Scheduler::run(const std::vector<Process>& processes_in, Algo algo, double quantum, double mlfq_base_quantum, bool verbose, std::vector<ScheduleEvent>& events_out) {
//     // copy and initialize
//     std::vector<Process> procs = processes_in;
//     for (auto &p : procs) {
//         p.remaining = p.burst;
//         p.first_response_time = -1.0;
//         p.finish_time = -1.0;
//     }
//     std::sort(procs.begin(), procs.end(), [](const Process& a, const Process& b){
//         if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
//         return a.pid < b.pid;
//     });

//     double current_time = 0.0;
//     double cpu_busy_time = 0.0;
//     size_t next_arrival_idx = 0;
//     const size_t N = procs.size();
//     int context_switches = 0;
//     int last_pid = -1;

//     // helper to add arrivals up to time t into ready container
//     auto add_arrivals = [&](double t, std::vector<int>& ready_indices) {
//         while (next_arrival_idx < N && procs[next_arrival_idx].arrival_time <= t + 1e-9) {
//             ready_indices.push_back((int)next_arrival_idx);
//             ++next_arrival_idx;
//         }
//     };

//     // initialize time to first arrival
//     if (next_arrival_idx < N) current_time = procs[next_arrival_idx].arrival_time;
//     // a small ready list we will adapt per algo
//     std::vector<int> ready;

//     // main loop (discrete-event)
//     while (true) {
//         // stop condition
//         if (all_done(procs)) break;

//         // refill ready from arrivals up to current_time
//         add_arrivals(current_time, ready);

//         // if no ready tasks, jump to next arrival
//         if (ready.empty()) {
//             if (next_arrival_idx < N) {
//                 current_time = procs[next_arrival_idx].arrival_time;
//                 add_arrivals(current_time, ready);
//             } else {
//                 break;
//             }
//         }

//         // choose next process index depending on algo
//         int chosen = -1;
//         double run_for = 0.0;

//         if (algo == Algo::FCFS) {
//             // pick earliest arrival among ready (they are by index order)
//             std::sort(ready.begin(), ready.end(), [&](int a, int b){
//                 if (procs[a].arrival_time != procs[b].arrival_time) return procs[a].arrival_time < procs[b].arrival_time;
//                 return procs[a].pid < procs[b].pid;
//             });
//             chosen = ready.front();
//             ready.erase(ready.begin());
//             run_for = procs[chosen].remaining; // run to completion
//         }
//         else if (algo == Algo::SJF_NONPREEMPTIVE) {
//             // choose ready with smallest burst (total or remaining -> use remaining)
//             chosen = -1;
//             double best = std::numeric_limits<double>::infinity();
//             int pos = -1;
//             for (size_t i = 0; i < ready.size(); ++i) {
//                 int idx = ready[i];
//                 if (procs[idx].remaining < best) { best = procs[idx].remaining; chosen = idx; pos = (int)i; }
//             }
//             if (pos != -1) ready.erase(ready.begin() + pos);
//             run_for = procs[chosen].remaining;
//         }
//         else if (algo == Algo::SJF_PREEMPTIVE) {
//             // preemptive SJF (SRPT): pick process with smallest remaining among ready
//             chosen = -1;
//             double best = std::numeric_limits<double>::infinity();
//             for (size_t i = 0; i < ready.size(); ++i) {
//                 int idx = ready[i];
//                 if (procs[idx].remaining < best) { best = procs[idx].remaining; chosen = idx; }
//             }
//             // compute next event: either it finishes or a new arrival comes earlier
//             double time_to_finish = procs[chosen].remaining;
//             double next_arrival_time = (next_arrival_idx < N) ? procs[next_arrival_idx].arrival_time : std::numeric_limits<double>::infinity();
//             run_for = std::min(time_to_finish, next_arrival_time - current_time);
//             if (run_for < 1e-9) { // if next arrival at same time, advance and continue
//                 current_time = next_arrival_time;
//                 continue;
//             }
//             // keep chosen in ready; we will update its remaining; if not finished, it stays or re-evaluated
//             // we remove chosen now and if not finished we will re-add it after updating
//             auto it = std::find(ready.begin(), ready.end(), chosen);
//             if (it != ready.end()) ready.erase(it);
//         }
//         else if (algo == Algo::RR) {
//             // Round Robin: maintain queue order in ready vector (FIFO)
//             // if ready not ordered as queue, sort by arrival index
//             // pick front
//             chosen = ready.front();
//             ready.erase(ready.begin());
//             run_for = std::min(quantum, procs[chosen].remaining);
//         }
//         else if (algo == Algo::MLFQ) {
//             // Simple MLFQ: implement 3 levels using different quantums
//             // For simplicity, use round-robin per level. We'll maintain a small structure:
//             static std::vector<std::deque<int>> levels;
//             static bool initialized = false;
//             if (!initialized) {
//                 levels.clear();
//                 levels.resize(3);
//                 // seed initial ready tasks into highest level
//                 for (int idx : ready) levels[0].push_back(idx);
//                 ready.clear();
//                 initialized = true;
//             } else {
//                 // push newly arrived into top level
//                 for (int idx : ready) levels[0].push_back(idx);
//                 ready.clear();
//             }
//             // pick highest non-empty level
//             int level = -1;
//             for (size_t i = 0; i < levels.size(); ++i) if (!levels[i].empty()) { level = (int)i; break; }
//             if (level == -1) {
//                 // nothing in levels, maybe arrivals later; continue
//                 if (next_arrival_idx < N) {
//                     current_time = procs[next_arrival_idx].arrival_time;
//                     add_arrivals(current_time, ready);
//                     for (int idx : ready) levels[0].push_back(idx);
//                     ready.clear();
//                     continue;
//                 } else break;
//             }
//             chosen = levels[level].front();
//             levels[level].pop_front();
//             double q = mlfq_base_quantum * std::pow(2.0, (double)level);
//             run_for = std::min(q, procs[chosen].remaining);
//             // after run, if still remaining -> demote to next level
//             // we will handle after execution
//         }
//         else {
//             // unknown algorithm
//             break;
//         }

//         // set first response time if needed
//         if (procs[chosen].first_response_time < 0.0) {
//             procs[chosen].first_response_time = current_time - procs[chosen].arrival_time;
//         }

//         // count context switch if PID changed
//         if (last_pid != -1 && last_pid != procs[chosen].pid) context_switches++;
//         last_pid = procs[chosen].pid;

//         // record event
//         ScheduleEvent ev{ current_time, procs[chosen].pid, run_for, "" };
//         if (algo == Algo::RR) ev.queue = "RR";
//         else if (algo == Algo::MLFQ) ev.queue = "MLFQ";
//         else if (algo == Algo::FCFS) ev.queue = "FCFS";
//         else if (algo == Algo::SJF_NONPREEMPTIVE || algo == Algo::SJF_PREEMPTIVE) ev.queue = "SJF";

//         events_out.push_back(ev);

//         // advance time, update cpu busy time and remaining
//         current_time += run_for;
//         cpu_busy_time += run_for;
//         procs[chosen].remaining -= run_for;
//         if (procs[chosen].remaining < 1e-9) {
//             procs[chosen].finish_time = current_time;
//         }

//         // for preemptive SJF: if not finished, put back into ready for reconsideration
//         if (algo == Algo::SJF_PREEMPTIVE && procs[chosen].remaining > 1e-9) {
//             add_arrivals(current_time, ready); // new arrivals may have appeared
//             ready.push_back(chosen);
//             continue;
//         }

//         // For RR: if not finished, re-queue at end
//         if (algo == Algo::RR && procs[chosen].remaining > 1e-9) {
//             add_arrivals(current_time, ready); // insert newly arrived before queueing to ensure fairness
//             ready.push_back(chosen);
//         }

//         // For MLFQ: if not finished, demote to next level
//         if (algo == Algo::MLFQ && procs[chosen].remaining > 1e-9) {
//             // we used static 'levels' above; find current level (we don't track per-process level persistently here)
//             // For simplicity, push to lowest level when preempted
//             // Find the global levels vector by reusing static var via function scope — hacky but works.
//             // A more robust implementation should be class member. Keep simple now:
//             // We'll push to level 1 or 2 depending on where we came from
//             // (for robust behavior, add level tracking in Process struct)
//             // For simplicity push to lowest level now:
//             extern std::vector<std::deque<int>> *__mlfq_levels_ptr;
//             if (__mlfq_levels_ptr) {
//                 int next_level = std::min((int)__mlfq_levels_ptr->size()-1, 1);
//                 __mlfq_levels_ptr->at(next_level).push_back(chosen);
//             }
//         }

//         // Add any arrivals that came at or before current_time
//         add_arrivals(current_time, ready);
//     } // end main loop

//     // compute total finish time (max finish)
//     double total_finish = 0.0;
//     for (const auto& p : procs) if (p.finish_time > total_finish) total_finish = p.finish_time;
//     Metrics m = compute_metrics(procs, total_finish, cpu_busy_time, context_switches);

//     return m;
// }




#include "scheduler.h"
#include <algorithm>
#include <numeric>
#include <queue>
#include <iostream>

Scheduler::Scheduler(std::string algorithm, int tq) : algo(algorithm), time_quantum(tq) {}

SchedulerResult Scheduler::run(std::vector<Process>& processes) {
    if (algo == "FCFS") return runFCFS(processes);
    if (algo == "SJF")  return runSJF(processes);
    if (algo == "RR")   return runRR(processes);
    if (algo == "MLFQ") return runMLFQ(processes);
    throw std::runtime_error("Unknown algorithm: " + algo);
}

// Example: FCFS Implementation
SchedulerResult Scheduler::runFCFS(std::vector<Process>& processes) {
    std::sort(processes.begin(), processes.end(),
              [](const Process& a, const Process& b) { return a.arrival_time < b.arrival_time; });

    double current_time = 0;
    double total_wait = 0, total_turnaround = 0;
    int context_switches = 0;

    for (auto& p : processes) {
        if (current_time < p.arrival_time) current_time = p.arrival_time;
        p.start_time = current_time;
        p.completion_time = current_time + p.burst_time;
        p.waiting_time = p.start_time - p.arrival_time;
        p.turnaround_time = p.completion_time - p.arrival_time;

        total_wait += p.waiting_time;
        total_turnaround += p.turnaround_time;
        current_time = p.completion_time;
        context_switches++;
    }

    SchedulerResult result;
    result.avg_waiting_time = total_wait / processes.size();
    result.avg_turnaround_time = total_turnaround / processes.size();
    result.throughput = processes.size() / current_time;
    result.cpu_utilization = (current_time > 0) ? (100.0) : 0;
    result.context_switches = context_switches;
    //result.gantt_chart = processes;
    return result;
}

// Implement runSJF, runRR, runMLFQ similarly
SchedulerResult Scheduler::runSJF(std::vector<Process>& processes){
     SchedulerResult result{};
    int time = 0;
    int completed = 0;
    int totalBurst = 0;
    result.context_switches = 0;

    for (auto& p : processes) totalBurst += p.burst_time;

    std::vector<bool> done(processes.size(), false);

    while (completed < processes.size()) {
        // Find process with smallest burst among arrived processes
        int idx = -1;
        int minBurst = INT_MAX;
        for (int i = 0; i < processes.size(); i++) {
            if (!done[i] && processes[i].arrival_time <= time &&
                processes[i].burst_time < minBurst) {
                minBurst = processes[i].burst_time;
                idx = i;
            }
        }

        if (idx == -1) {
            time++;
            continue;  // CPU idle
        }

        Process& p = processes[idx];
        //result.gantt_chart.push_back({p.pid, time, p.burst_time});
        result.gantt_chart.push_back(GanttEvent{p.pid, time, p.burst_time});

        //

        int start_time = time;
        time += p.burst_time;
        int finish_time = time;

        p.waiting_time = start_time - p.arrival_time;
        p.turnaround_time = finish_time - p.arrival_time;

        done[idx] = true;
        completed++;
        result.context_switches++;
    }

    double totalWT = 0, totalTAT = 0;
    for (const auto& p : processes) {
        totalWT += p.waiting_time;
        totalTAT += p.turnaround_time;
    }

    result.avg_waiting_time = totalWT / processes.size();
    result.avg_turnaround_time = totalTAT / processes.size();
    result.cpu_utilization = (double)totalBurst / time * 100.0;
    result.throughput = (double)processes.size() / time;

    return result;

}
SchedulerResult Scheduler::runRR(std::vector<Process>& processes){
    // Placeholder for RR implementation
     SchedulerResult result{};
    int quantum = 4; // You can parameterize this
    int time = 0;
    int totalBurst = 0;
    result.context_switches = 0;

    for (auto& p : processes) totalBurst += p.burst_time;

    std::queue<int> readyQueue;
    std::vector<int> remaining(processes.size());
    for (int i = 0; i < processes.size(); i++) remaining[i] = processes[i].burst_time;

    int completed = 0;
    int lastTime = 0;

    while (completed < processes.size()) {
        // Push newly arrived processes into queue
        for (int i = 0; i < processes.size(); i++) {
            if (processes[i].arrival_time <= time && remaining[i] > 0 &&
                std::find_if(readyQueue._Get_container().begin(),
                             readyQueue._Get_container().end(),
                             [&](int x) { return x == i; }) == readyQueue._Get_container().end()) {
                readyQueue.push(i);
            }
        }

        if (readyQueue.empty()) {
            time++;
            continue;
        }

        int idx = readyQueue.front();
        readyQueue.pop();

        int run_time = std::min(quantum, remaining[idx]);
        result.gantt_chart.push_back({processes[idx].pid, time, run_time});

        remaining[idx] -= run_time;
        time += run_time;
        result.context_switches++;

        if (remaining[idx] == 0) {
            completed++;
            processes[idx].turnaround_time = time - processes[idx].arrival_time;
            processes[idx].waiting_time =
                processes[idx].turnaround_time - processes[idx].burst_time;
        } else {
            readyQueue.push(idx);  // Requeue if not finished
        }
    }

    double totalWT = 0, totalTAT = 0;
    for (const auto& p : processes) {
        totalWT += p.waiting_time;
        totalTAT += p.turnaround_time;
    }

    result.avg_waiting_time = totalWT / processes.size();
    result.avg_turnaround_time = totalTAT / processes.size();
    result.cpu_utilization = (double)totalBurst / time * 100.0;
    result.throughput = (double)processes.size() / time;

    return result;

}
SchedulerResult Scheduler::runMLFQ(std::vector<Process>& processes){
    SchedulerResult result{};
    int time = 0;
    result.context_switches = 0;

    // Copy remaining burst times
    std::vector<int> remaining(processes.size());
    for (int i = 0; i < processes.size(); i++) remaining[i] = processes[i].burst_time;

    std::queue<int> q1, q2, q3;
    int completed = 0;
    int totalBurst = 0;
    for (auto& p : processes) totalBurst += p.burst_time;

    auto enqueueArrivals = [&](int current_time) {
        for (int i = 0; i < processes.size(); i++) {
            if (processes[i].arrival_time <= current_time && remaining[i] > 0 &&
                std::find_if(q1._Get_container().begin(), q1._Get_container().end(),
                             [&](int x) { return x == i; }) == q1._Get_container().end() &&
                std::find_if(q2._Get_container().begin(), q2._Get_container().end(),
                             [&](int x) { return x == i; }) == q2._Get_container().end() &&
                std::find_if(q3._Get_container().begin(), q3._Get_container().end(),
                             [&](int x) { return x == i; }) == q3._Get_container().end()) {
                q1.push(i);
            }
        }
    };

    while (completed < processes.size()) {
        enqueueArrivals(time);
        int idx = -1;
        int quantum = 0;

        if (!q1.empty()) {
            idx = q1.front(); q1.pop();
            quantum = 4;
        } else if (!q2.empty()) {
            idx = q2.front(); q2.pop();
            quantum = 8;
        } else if (!q3.empty()) {
            idx = q3.front(); q3.pop();
            quantum = remaining[idx]; // FCFS
        } else {
            time++;
            continue;
        }

        int run_time = std::min(quantum, remaining[idx]);
        result.gantt_chart.push_back({processes[idx].pid, time, run_time});

        remaining[idx] -= run_time;
        time += run_time;
        result.context_switches++;

        if (remaining[idx] == 0) {
            completed++;
            processes[idx].turnaround_time = time - processes[idx].arrival_time;
            processes[idx].waiting_time =
                processes[idx].turnaround_time - processes[idx].burst_time;
        } else {
            if (quantum == 4) q2.push(idx);
            else if (quantum == 8) q3.push(idx);
            else q3.push(idx);
        }
    }

    double totalWT = 0, totalTAT = 0;
    for (const auto& p : processes) {
        totalWT += p.waiting_time;
        totalTAT += p.turnaround_time;
    }

    result.avg_waiting_time = totalWT / processes.size();
    result.avg_turnaround_time = totalTAT / processes.size();
    result.cpu_utilization = (double)totalBurst / time * 100.0;
    result.throughput = (double)processes.size() / time;

    return result;
}
