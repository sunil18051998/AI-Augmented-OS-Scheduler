#include "scheduler.h"
#include <queue>
#include <deque>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <nlohmann/json.hpp> // used only for pretty printing metrics (optional). If you don't want this dep, we output a simple JSON string manually.

using json = nlohmann::json;

Scheduler::Scheduler(std::shared_ptr<PredictorInterface> predictor) : predictor(predictor) {}

/*
 Simulation strategy (discrete-event, time advances to either next arrival or after a scheduled runtime):
 - Keep vector of not-yet-arrived processes (sorted by arrival_time).
 - Maintain ready queues depending on selected algorithm.
 - When CPU idle and no ready process, advance time to next arrival.
 - When CPU runs a process, generate a ScheduleEvent and decrement remaining.
 - Track per-process first-response and finish times for metrics.
*/

Metrics Scheduler::run(const std::vector<Process>& processes_in, Algo algo, double quantum, double mlfq_base_quantum, bool verbose, std::vector<ScheduleEvent>& events_out) {
    // copy processes so we can mutate remaining
    std::vector<Process> procs = processes_in;
    std::sort(procs.begin(), procs.end(), [](const Process& a, const Process& b){
        return a.arrival_time < b.arrival_time;
    });

    double current_time = 0.0;
    double cpu_busy_time = 0.0;
    size_t next_arrival_idx = 0;
    const size_t N = procs.size();
    // ready structures
    std::deque<int> rr_queue; // store indices into procs
    // MLFQ: 3 levels, level 0 highest priority (short quantum), demote to lower on use
    std::vector<std::deque<int>> mlfq_queues(3);

    auto push_ready = [&](int idx){
        if (algo == Algo::MLFQ) {
            mlfq_queues[0].push_back(idx); // new arrivals to highest priority
        } else {
            rr_queue.push_back(idx);
        }
    };

    // helper to populate arrivals up to current_time
    auto add_arrivals_up_to = [&](double t) {
        while (next_arrival_idx < N && procs[next_arrival_idx].arrival_time <= t + 1e-9) {
            int idx = (int)next_arrival_idx;
            procs[idx].remaining = procs[idx].burst;
            procs[idx].initial_priority = 0;
            push_ready(idx);
            ++next_arrival_idx;
        }
    };

    // ensure at start we move time to first arrival if nothing
    if (next_arrival_idx < N) current_time = procs[next_arrival_idx].arrival_time;
    add_arrivals_up_to(current_time);

    while (true) {
        // stop when all completed
        bool all_done = true;
        for (const auto& p : procs) if (p.remaining > 1e-9) { all_done = false; break; }
        if (all_done) break;

        // if no ready, jump to next arrival
        bool has_ready = false;
        if (algo == Algo::MLFQ) {
            for (auto &q : mlfq_queues) if (!q.empty()) { has_ready = true; break; }
        } else {
            has_ready = !rr_queue.empty();
        }
        if (!has_ready) {
            if (next_arrival_idx < N) {
                current_time = std::max(current_time, procs[next_arrival_idx].arrival_time);
                add_arrivals_up_to(current_time);
                continue;
            } else {
                break;
            }
        }

        int pick_idx = -1;
        double slot = quantum;
        std::string queue_name = (algo==Algo::MLFQ ? "MLFQ" : (algo==Algo::ML_RR ? "ML_RR" : "RR"));

        if (algo == Algo::RR) {
            pick_idx = rr_queue.front();
            rr_queue.pop_front();
            slot = quantum;
            queue_name = "RR";
        } else if (algo == Algo::ML_RR) {
            // select process from rr_queue with smallest predicted remaining
            if (rr_queue.empty()) { continue; }
            double best_pred = 1e18;
            int best_pos = 0;
            for (size_t i = 0; i < rr_queue.size(); ++i) {
                int idx = rr_queue[i];
                double fallback = procs[idx].remaining;
                double pred = predictor ? predictor->predict(procs[idx].features, fallback) : fallback;
                if (pred < best_pred) { best_pred = pred; best_pos = (int)i; }
            }
            pick_idx = rr_queue[best_pos];
            // remove from deque at best_pos
            rr_queue.erase(rr_queue.begin() + best_pos);
            slot = quantum;
        } else { // MLFQ
            // find highest non-empty queue
            int level = -1;
            for (size_t i = 0; i < mlfq_queues.size(); ++i) {
                if (!mlfq_queues[i].empty()) { level = (int)i; break; }
            }
            if (level == -1) continue;
            pick_idx = mlfq_queues[level].front();
            mlfq_queues[level].pop_front();
            // quantum per level: base * (2^level) for example
            slot = mlfq_base_quantum * std::pow(2.0, level);
            queue_name = "Q" + std::to_string(level);
        }

        // if process hasn't responded yet, set response time
        if (procs[pick_idx].first_response_time < 0.0) {
            procs[pick_idx].first_response_time = current_time - procs[pick_idx].arrival_time;
        }

        // optionally use predictor to decide how long to run this slice (advisory)
        double predicted = procs[pick_idx].remaining;
        if (predictor) {
            predicted = predictor->predict(procs[pick_idx].features, procs[pick_idx].remaining);
            // clamp
            if (predicted < 0.0) predicted = 0.0;
        }

        // scheduling policy: run for min(slot, remaining). You could incorporate predicted to adjust run length.
        double run_time = std::min(slot, procs[pick_idx].remaining);
        // but if predicted much smaller than slot and algorithm is ML_RR, we might run only predicted to reduce overshoot
        if (algo == Algo::ML_RR) {
            run_time = std::min(run_time, predicted);
            // ensure non-zero
            run_time = std::max(run_time, std::min(1e-6, procs[pick_idx].remaining));
        }

        // produce schedule event
        ScheduleEvent ev;
        ev.time = current_time;
        ev.pid = procs[pick_idx].pid;
        ev.duration = run_time;
        ev.queue = queue_name;
        events_out.push_back(ev);

        // verbose output to stdout as JSON line for easy consumption
        if (verbose) {
            // small manual JSON for portability (no external deps)
            std::cout << "{\"type\":\"schedule_event\",\"time\":" << ev.time
                      << ",\"pid\":" << ev.pid << ",\"duration\":" << ev.duration
                      << ",\"queue\":\"" << ev.queue << "\"}" << std::endl;
        }

        // advance time and update process remaining
        current_time += run_time;
        cpu_busy_time += run_time;
        procs[pick_idx].remaining -= run_time;

        // mark finished if done
        if (procs[pick_idx].remaining <= 1e-9) {
            procs[pick_idx].finish_time = current_time;
        } else {
            // not finished: re-queue according to algorithm
            if (algo == Algo::RR || algo == Algo::ML_RR) {
                rr_queue.push_back(pick_idx);
            } else { // MLFQ: demote to next level (if not already at bottom)
                int cur_level = procs[pick_idx].initial_priority; // we can track last level in initial_priority
                int next_level = std::min((int)mlfq_queues.size()-1, cur_level + 1);
                procs[pick_idx].initial_priority = next_level;
                mlfq_queues[next_level].push_back(pick_idx);
            }
        }

        // add any newly arrived processes that arrived up to current_time
        while (next_arrival_idx < N && procs[next_arrival_idx].arrival_time <= current_time + 1e-9) {
            int idx = (int)next_arrival_idx;
            procs[idx].remaining = procs[idx].burst;
            procs[idx].initial_priority = 0;
            if (algo == Algo::MLFQ) mlfq_queues[0].push_back(idx);
            else rr_queue.push_back(idx);
            ++next_arrival_idx;
        }
    } // end main loop

    // compute metrics
    // total time is max finish_time among procs
    double total_finish = 0.0;
    for (const auto& p : procs) if (p.finish_time > total_finish) total_finish = p.finish_time;
    Metrics m = compute_metrics(procs, total_finish, cpu_busy_time);

    // print metrics JSON (if verbose)
    if (verbose) {
        // simple JSON output
        std::cout << "{\"type\":\"metrics\",\"processed\":" << m.processed
                  << ",\"avg_turnaround\":" << m.avg_turnaround
                  << ",\"avg_waiting\":" << m.avg_waiting
                  << ",\"avg_response\":" << m.avg_response
                  << ",\"cpu_utilization\":" << m.cpu_utilization << "}" << std::endl;
    }

    return m;
}

Metrics Scheduler::compute_metrics(const std::vector<Process>& procs, double total_time, double cpu_busy_time) {
    Metrics m;
    double sum_tat = 0.0, sum_wait = 0.0, sum_resp = 0.0;
    int count = 0;
    for (const auto& p : procs) {
        if (p.finish_time < 0.0) continue;
        double tat = p.finish_time - p.arrival_time;
        double wait = tat - p.burst;
        double resp = p.first_response_time;
        sum_tat += tat;
        sum_wait += wait;
        sum_resp += resp;
        ++count;
    }
    if (count > 0) {
        m.processed = count;
        m.avg_turnaround = sum_tat / count;
        m.avg_waiting = sum_wait / count;
        m.avg_response = sum_resp / count;
    }
    m.cpu_utilization = total_time > 0.0 ? (cpu_busy_time / total_time) * 100.0 : 0.0;
    return m;
}
