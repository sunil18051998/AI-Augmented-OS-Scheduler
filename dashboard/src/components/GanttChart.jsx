// src/components/GanttChart.jsx
import { useEffect, useRef } from "react";
import * as d3 from "d3";

export default function GanttChart({ events }) {
  const ref = useRef();

  useEffect(() => {
    if (!events.length) return;
    const svg = d3.select(ref.current);
    svg.selectAll("*").remove();

    const width = 800;
    const height = 300;

    const x = d3
      .scaleLinear()
      .domain([0, d3.max(events, (d) => d.time + d.duration)])
      .range([50, width - 50]);

    const y = d3
      .scaleBand()
      .domain([...new Set(events.map((d) => d.pid))])
      .range([50, height - 50])
      .padding(0.2);

    // axes
    svg
      .append("g")
      .attr("transform", `translate(0, ${height - 50})`)
      .call(d3.axisBottom(x));
    svg.append("g").attr("transform", "translate(50,0)").call(d3.axisLeft(y));

    // bars (process execution)
    svg
      .selectAll("rect")
      .data(events)
      .enter()
      .append("rect")
      .attr("x", (d) => x(d.time))
      .attr("y", (d) => y(d.pid))
      .attr("width", (d) => x(d.time + d.duration) - x(d.time))
      .attr("height", y.bandwidth())
      .attr("fill", "#4CAF50")
      .append("title")
      .text((d) => `PID: ${d.pid}, duration: ${d.duration}`);
  }, [events]);

  return <svg ref={ref} width={900} height={400}></svg>;
}
