import express from "express";
import cors from "cors";
import bodyParser from "body-parser";
import { WebSocketServer } from "ws";
import { execFile } from "child_process";

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
  const { algorithm, dataset, quantum } = req.body;
  const args = [algorithm, dataset];
  if (quantum) args.push(String(quantum));
  execFile("../bin/test_predictor.exe", args, (err, stdout, stderr) => {
    if (err) return res.status(500).json({ error: stderr || err.message });
    try {
      const parsed = JSON.parse(stdout);
      return res.json(parsed);
    } catch (e) {
      return res.status(500).json({ error: "Invalid JSON from core", raw: stdout });
    }
  });
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
