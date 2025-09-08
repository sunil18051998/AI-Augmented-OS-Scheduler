// src/api.js
import axios from "axios";

const API_URL = "http://localhost:5000"; // scheduler backend

export const getAlgorithms = async () => {
  const res = await axios.get(`${API_URL}/algorithms`);
  return res.data;
};

export const startSimulation = async (algo) => {
  const res = await axios.post(`${API_URL}/simulate`, { algorithm: algo });
  return res.data;
};

// WebSocket connection for live events
export const connectEvents = (onEvent) => {
  const ws = new WebSocket("ws://localhost:5000/events");
  ws.onmessage = (msg) => {
    const data = JSON.parse(msg.data);
    onEvent(data);
  };
  return ws;
};
