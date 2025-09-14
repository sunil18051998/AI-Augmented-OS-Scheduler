import { useEffect, useRef } from "react";
import * as d3 from "d3";

export default function GanttChart({ events }) {
  console.log("[GanttChart] Received events:", events);
  const ref = useRef();

  useEffect(() => {
    if (!Array.isArray(events) || events.length === 0) return;

    // Flatten and parse events from 'core_log' type only
    let safeEvents = [];

    events.forEach((e) => {
      if (e.type === "core_log" && typeof e.message === "string") {
        try {
          const parsed = JSON.parse(e.message); // message is a JSON string array
          parsed.forEach((d) => {
            const time = Number(d.time);
            const duration = Number(d.duration);
            if (
              d.pid &&
              Number.isFinite(time) &&
              Number.isFinite(duration) &&
              duration > 0
            ) {
              safeEvents.push({ pid: d.pid, time, duration });
            }
          });
        } catch (err) {
          console.warn("[GanttChart] Failed to parse message:", e.message, err);
        }
      }
    });

    if (safeEvents.length === 0) {
      console.warn("[GanttChart] No valid events to render.");
      return;
    }

    console.log("[GanttChart] Rendering events:", safeEvents);

    const svg = d3.select(ref.current);
    svg.selectAll("*").remove();

    const width = 800;
    const height = 300;

    const maxTime = d3.max(safeEvents, (d) => d.time + d.duration) || 0;

    const x = d3.scaleLinear().domain([0, maxTime]).range([50, width - 50]);

    const pids = [...new Set(safeEvents.map((d) => d.pid))];

    const y = d3.scaleBand().domain(pids).range([50, height - 50]).padding(0.2);

    // axes
    svg
      .append("g")
      .attr("transform", `translate(0, ${height - 50})`)
      .call(d3.axisBottom(x));
    svg.append("g").attr("transform", "translate(50,0)").call(d3.axisLeft(y));

    // bars
    svg
      .selectAll("rect")
      .data(safeEvents)
      .enter()
      .append("rect")
      .attr("x", (d) => x(d.time))
      .attr("y", (d) => y(d.pid))
      .attr("width", (d) => Math.max(1, x(d.time + d.duration) - x(d.time)))
      .attr("height", y.bandwidth())
      .attr("fill", "#4CAF50")
      .append("title")
      .text((d) => `PID: ${d.pid}, duration: ${d.duration}`);
  }, [events]);

  return <svg ref={ref} width={900} height={400}></svg>;
}
