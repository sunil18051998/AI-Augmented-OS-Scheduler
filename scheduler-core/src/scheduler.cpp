#include "scheduler.h"
#include <queue>
#include <iostream>


Scheduler::Scheduler(std::shared_ptr<PredictorInterface> pred) : predictor(pred) {}


void Scheduler::run_simulation(const std::vector<Process>& processes) {
// discrete-event simulation loop: maintain current_time, event queue etc.
// For simplicity: implement a ready-queue driven simulation that uses arrival_time
schedule_rr();
}


void Scheduler::schedule_rr() {
// Example: use quantum = 10ms
const double quantum = 10.0;
// PSEUDO: iterate ready queue, call predictor->predict(process.features) to get predicted burst
}


void Scheduler::schedule_mlfq() {
// Implement multi-level feedback queues and demotion/promotion logic
}