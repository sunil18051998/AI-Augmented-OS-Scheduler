import csv
import random
import argparse

def generate_dataset(file_path, num_processes=100, max_arrival=20, max_burst=15, max_priority=5, max_io_time=10, max_memory_req=10):
    """
    Generates a random process dataset for scheduling algorithms.

    CSV Format:
    pid,arrival_time,burst_time,priority
    """
    processes = []

    for pid in range(1, num_processes + 1):
        arrival_time = random.randint(0, max_arrival)
        burst_time = random.randint(1, max_burst)
        priority = random.randint(1, max_priority)
        io_time = random.randint(1, max_io_time)
        memory_req = random.randint(1, max_memory_req)

        processes.append({
            "pid": pid,
            "arrival_time": arrival_time,
            "burst_time": burst_time,
            "priority": priority,
            "io_time": io_time,
            "memory_req": memory_req
        })

    # Sort by arrival time for realism
    processes.sort(key=lambda p: p["arrival_time"])

    with open(file_path, mode="w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=["pid", "arrival_time", "burst_time", "priority", "io_time","memory_req"])
        writer.writeheader()
        writer.writerows(processes)

    print(f"✅ Dataset generated: {file_path}")
    print(f"Processes: {len(processes)}")
    print("Sample rows:")
    for p in processes[:5]:
        print(p)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate process dataset for scheduler testing.")
    parser.add_argument("--file", type=str, default="test2.csv", help="Output CSV file path")
    parser.add_argument("--n", type=int, default=100, help="Number of processes")
    parser.add_argument("--max-arrival", type=int, default=20, help="Max arrival time")
    parser.add_argument("--max-burst", type=int, default=15, help="Max burst time")
    parser.add_argument("--max-priority", type=int, default=5, help="Max priority value")
    parser.add_argument("--max-io-time", type=int, default=10, help="Max I/O wait time")
    parser.add_argument("--max-memory-req", type=int, default=10, help="Max memory request")
    
    args = parser.parse_args()

    generate_dataset(args.file, args.n, args.max_arrival, args.max_burst, args.max_priority, args.max_io_time, args.max_memory_req)
