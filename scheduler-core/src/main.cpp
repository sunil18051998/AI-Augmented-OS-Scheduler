// #include "scheduler.h"
// #include "predictor.h"
// #include <iostream>
// #include <vector>
// #include <string>
// #include <sstream>
// #include <cstdlib>

// // small helper: create synthetic workload
// std::vector<Process> make_synthetic_workload(int n) {
//     std::vector<Process> v;
//     double t = 0.0;
//     for (int i = 0; i < n; ++i) {
//         Process p;
//         p.pid = i + 1;
//         // Poisson-like arrivals: vary arrival intervals somewhat
//         double inter = (i==0) ? 0.0 : (1.0 + (i % 3)); // simple varying arrivals
//         t += inter;
//         p.arrival_time = t;
//         // burst: mix CPU-bound and IO-bound
//         if (i % 3 == 0) p.burst = 50.0 + (i % 5) * 10.0; // CPU heavy
//         else if (i % 3 == 1) p.burst = 8.0 + (i % 4) * 2.0; // short
//         else p.burst = 20.0 + (i % 6) * 3.0;
//         p.remaining = p.burst;
//         // example features: [arrival, burst] - you can expand
//         p.features = {p.arrival_time, p.burst};
//         v.push_back(p);
//     }
//     return v;
// }

// Algo parse_algo(const std::string& s) {
//     if (s == "RR") return Algo::RR;
//     if (s == "MLFQ") return Algo::MLFQ;
//     if (s == "ML_RR") return Algo::ML_RR;
//     return Algo::RR;
// }

// int main(int argc, char** argv) {
//     int n = 12;
//     std::string algo_str = "RR";
//     double quantum = 10.0;
//     double mlfq_base = 5.0;
//     double predictor_noise = 0.0;
//     bool verbose = true;

//     // quick arg parsing
//     for (int i = 1; i < argc; ++i) {
//         std::string a = argv[i];
//         if (a == "--n" && i+1 < argc) { n = std::atoi(argv[++i]); }
//         else if (a == "--algo" && i+1 < argc) { algo_str = argv[++i]; }
//         else if (a == "--quantum" && i+1 < argc) { quantum = std::atof(argv[++i]); }
//         else if (a == "--mlfq_base" && i+1 < argc) { mlfq_base = std::atof(argv[++i]); }
//         else if (a == "--noise" && i+1 < argc) { predictor_noise = std::atof(argv[++i]); }
//         else if (a == "--quiet") { verbose = false; }
//     }

//     auto workload = make_synthetic_workload(n);
//     auto predictor = std::make_shared<DummyPredictor>(predictor_noise);

//     Scheduler sched(predictor);
//     std::vector<ScheduleEvent> events;
//     Algo algo = parse_algo(algo_str);

//     Metrics m = sched.run(workload, algo, quantum, mlfq_base, verbose, events);

//     // print final metrics to stdout as JSON if not already printed
//     if (!verbose) {
//         std::cout << "{\"type\":\"metrics\",\"processed\":" << m.processed
//                   << ",\"avg_turnaround\":" << m.avg_turnaround
//                   << ",\"avg_waiting\":" << m.avg_waiting
//                   << ",\"avg_response\":" << m.avg_response
//                   << ",\"cpu_utilization\":" << m.cpu_utilization << "}" << std::endl;
//     }

//     // exit
//     return 0;
// }



#include "onnx_predictor.h"
#include <iostream>
#include <vector>

int main() {
    try {
        // Path to your exported ONNX model (adjust if needed)
        std::string model_path = "../ml-predictor/burst_model.onnx";

        // Initialize predictor
        ONNXPredictor predictor(model_path);

        // Example features: [arrival, priority, io_wait, prev_burst]
        std::vector<float> features = {100.0f, 2.0f, 15.0f, 7.0f};

        // Run inference
        float prediction = predictor.predictBurst(features);

        std::cout << "Predicted burst time: " << prediction << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
