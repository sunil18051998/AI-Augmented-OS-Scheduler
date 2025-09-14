// src/App.jsx
import { useEffect, useState } from "react";
import { getAlgorithms, startSimulation, connectEvents } from "./api";
import GanttChart from "./components/GanttChart";
import MetricsChart from "./components/MetricsChart";

export default function App() {
  const [algorithms, setAlgorithms] = useState([]);
  const [selectedAlgo, setSelectedAlgo] = useState("");
  const [events, setEvents] = useState([]);

  useEffect(() => {
    getAlgorithms().then(setAlgorithms);
    const ws = connectEvents((evt) => setEvents((prev) => [...prev, evt]));
    //return () => ws.close();
  }, []);

  const handleStart = () => {
    if (selectedAlgo) {
      setEvents([]);
      startSimulation(selectedAlgo);
    }
  };

  return (
    <div className="p-4">
      <h1 className="text-xl font-bold mb-2">AI-Augmented OS Scheduler Dashboard</h1>

      <select
        value={selectedAlgo}
        onChange={(e) => setSelectedAlgo(e.target.value)}
        className="border px-2 py-1 mr-2"
      >
        <option value="">Select Algorithm</option>
        {algorithms.map((a) => (
          <option key={a.id} value={a.name}>
            {a.name}
          </option>
        ))}
      </select>
      <button
        onClick={handleStart}
        className="bg-blue-500 text-white px-3 py-1 rounded"
      >
        Start Simulation
      </button>

      <div className="mt-6">
        <GanttChart events={events} />
        <MetricsChart events={events} />

      </div>
    </div>
  );
}
