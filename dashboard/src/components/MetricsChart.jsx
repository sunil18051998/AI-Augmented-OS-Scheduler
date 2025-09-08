// src/components/MetricsChart.jsx
import { useEffect, useState } from "react";
import {
  LineChart,
  Line,
  CartesianGrid,
  XAxis,
  YAxis,
  Tooltip,
  Legend,
} from "recharts";

export default function MetricsChart({ events }) {
  const [metrics, setMetrics] = useState([]);

  useEffect(() => {
    if (!events.length) return;

    // Compute metrics dynamically as events stream in
    const procFinishTimes = {};
    const procStartTimes = {};
    const procBursts = {};

    events.forEach((e) => {
      // track first start time
      if (!(e.pid in procStartTimes)) {
        procStartTimes[e.pid] = e.time;
      }
      // accumulate burst
      procBursts[e.pid] = (procBursts[e.pid] || 0) + e.duration;
      // track last finish time
      procFinishTimes[e.pid] = e.time + e.duration;
    });

    const newMetrics = Object.keys(procFinishTimes).map((pid) => {
      const arrival = procStartTimes[pid];
      const finish = procFinishTimes[pid];
      const burst = procBursts[pid];
      const tat = finish - arrival;
      const wt = tat - burst;
      return { pid, turnaround: tat, waiting: wt };
    });

    // aggregate averages
    const avgTat =
      newMetrics.reduce((s, m) => s + m.turnaround, 0) / newMetrics.length;
    const avgWt =
      newMetrics.reduce((s, m) => s + m.waiting, 0) / newMetrics.length;

    // CPU utilization
    const totalFinish = Math.max(...Object.values(procFinishTimes));
    const cpuBusy = events.reduce((s, e) => s + e.duration, 0);
    const cpuUtil = (cpuBusy / totalFinish) * 100;

    setMetrics([
      { name: "Avg Turnaround", value: avgTat },
      { name: "Avg Waiting", value: avgWt },
      { name: "CPU Utilization (%)", value: cpuUtil },
    ]);
  }, [events]);

  return (
    <div className="mt-6">
      <h2 className="text-lg font-semibold mb-2">Scheduler Metrics</h2>
      <LineChart width={600} height={300} data={metrics}>
        <Line type="monotone" dataKey="value" stroke="#8884d8" />
        <CartesianGrid stroke="#ccc" />
        <XAxis dataKey="name" />
        <YAxis />
        <Tooltip />
        <Legend />
      </LineChart>
    </div>
  );
}
