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
    if (!Array.isArray(events) || events.length === 0) return;

    // Flatten and parse core_log events
    const flatEvents = [];

    events.forEach((e) => {
      if (e.type === "core_log" && typeof e.message === "string") {
        try {
          const parsed = JSON.parse(e.message); // parse JSON string
          parsed.forEach((d) => {
            const time = Number(d.time);
            const duration = Number(d.duration);
            if (
              d.pid &&
              Number.isFinite(time) &&
              Number.isFinite(duration) &&
              duration > 0
            ) {
              flatEvents.push({ pid: d.pid, time, duration });
            }
          });
        } catch (err) {
          console.warn("[MetricsChart] Failed to parse message:", e.message, err);
        }
      }
    });

    if (flatEvents.length === 0) {
      console.warn("[MetricsChart] No valid events to compute metrics.");
      return;
    }

    // Compute metrics
    const procFinishTimes = {};
    const procStartTimes = {};
    const procBursts = {};

    flatEvents.forEach((e) => {
      if (!(e.pid in procStartTimes)) procStartTimes[e.pid] = e.time;
      procBursts[e.pid] = (procBursts[e.pid] || 0) + e.duration;
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

    // Aggregate averages
    const avgTat =
      newMetrics.reduce((s, m) => s + m.turnaround, 0) / newMetrics.length;
    const avgWt =
      newMetrics.reduce((s, m) => s + m.waiting, 0) / newMetrics.length;

    // CPU utilization
    const totalFinish = Math.max(...Object.values(procFinishTimes));
    const cpuBusy = flatEvents.reduce((s, e) => s + e.duration, 0);
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
