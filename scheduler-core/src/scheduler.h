#pragma once
#include "process.h"
#include <vector>
#include <memory>


class PredictorInterface {
public:
virtual double predict(const std::vector<double>& features) = 0;
virtual ~PredictorInterface() {}
};


class Scheduler {
public:
Scheduler(std::shared_ptr<PredictorInterface> pred);
void run_simulation(const std::vector<Process>& processes);
private:
std::shared_ptr<PredictorInterface> predictor;
void schedule_rr();
void schedule_mlfq();
};