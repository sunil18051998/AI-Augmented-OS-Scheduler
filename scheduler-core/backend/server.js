import express from "express";
import cors from "cors";
import bodyParser from "body-parser";
import { WebSocketServer } from "ws";
import { spawn } from "child_process";

const app = express();
const PORT = 5000;

app.use(cors());
app.use(bodyParser.json());

let wsClients = new Set(); // Track connected WebSocket clients

// Sample: Return available algorithms
app.get("/algorithms", (req, res) => {
  res.json([
    { id: "fcfs", name: "First Come First Serve" },
    { id: "sjf", name: "Shortest Job First" },
    { id: "rr", name: "Round Robin" },
    { id: "mlfq", name: "Multi-Level Feedback Queue" },
  ]);
});

// Start simulation using C++ core
app.post("/simulate", (req, res) => {
  const { algorithm } = req.body;
  console.log(`[BACKEND] Starting simulation with algorithm: ${algorithm}`);

  // Spawn C++ core (binary must be built and available)
  const coreProcess = spawn("../../scheduler-core/bin/test_predictor.exe", [], {
    cwd: process.cwd(),
  });

  coreProcess.stdout.on("data", (data) => {
    const message = data.toString().trim();
    console.log(`[CORE OUTPUT] ${message}`);

    // Example: send live events to frontend
    wsClients.forEach((client) => {
      client.send(JSON.stringify({ type: "core_log", message }));
    });
  });

  coreProcess.stderr.on("data", (err) => {
    console.error(`[CORE ERROR] ${err}`);
  });

  coreProcess.on("close", (code) => {
    console.log(`[CORE EXIT] Process exited with code ${code}`);
    wsClients.forEach((client) => {
      client.send(JSON.stringify({ type: "simulation_done", code }));
    });
  });

  res.json({ status: "simulation started", algorithm });
});

// Start HTTP server
const server = app.listen(PORT, () =>
  console.log(`[BACKEND] Server running on http://localhost:${PORT}`)
);

// WebSocket server for events
const wss = new WebSocketServer({ server, path: "/events" });

wss.on("connection", (ws) => {
  console.log("[WS] Client connected");
  wsClients.add(ws);

  ws.on("close", () => {
    console.log("[WS] Client disconnected");
    wsClients.delete(ws);
  });
});
