# Multi-Level Feedback Queue (MLFQ) Scheduler for Dual Processor Systems

A high-performance MLFQ-based scheduler designed for dual processor systems with dynamic load balancing, inter-processor cooperation, and detailed performance analytics.

---

##  Table of Contents

* [Overview](#overview)
* [Features](#features)
* [System Design](#system-design)
* [Queue Configuration](#queue-configuration)
* [Scheduling Algorithm](#scheduling-algorithm)
* [Build & Run Instructions](#build--run-instructions)
* [Usage](#usage)
* [Input Format](#input-format)
* [Output Format](#output-format)
* [Project Structure](#project-structure)
* [Contributing](#contributing)

---

##  Overview

This project implements a Multi-Level Feedback Queue (MLFQ) scheduler tailored for **dual processor systems**. It ensures optimal CPU usage through **smart load balancing**, **inter-processor assistance**, and **priority-based execution**. The scheduler supports detailed runtime analysis, producing Gantt charts and various performance metrics.

###  What is MLFQ?

MLFQ uses multiple priority queues where:

* Processes start in the highest queue.
* Non-completing processes are demoted.
* Lower queues follow more relaxed policies.

With dual processors, the scheduler supports **parallelism**, and **cooperative execution**, where idle CPUs contribute to ongoing tasks of busy CPUs.

---

##  Features

* Multi-tier MLFQ with priority-based demotion
* Dual processor handling with real-time load balancing
* Inter-CPU cooperation when one CPU is idle
* Preemptive scheduling with time quantum enforcement
* FCFS for the lowest queue
* Detailed Gantt chart visualization
* Complete performance summary including CPU utilization

---

##  System Design

```
[Process Input] → [Load Balancer] → [CPU 1] ←→ [CPU 2]
                                    ↓
                        [Performance Analyzer + Gantt Chart]
```

Each CPU maintains three queues:

```
┌──────────────┐
│  CPU Core X  │
├──────────────┤
│ Q1: Priority │ (TQ = 5 units)
│ Q2: Medium   │ (TQ = 8 units)
│ Q3: Low      │ (FCFS)
└──────────────┘
```

---

##  Queue Configuration

```

| Queue | Priority | Time Quantum | Policy      | Demotes To |
|-------|----------|--------------|-------------|------------|
| Q1    | High     | 5 units      | Round Robin | Q2         |
| Q2    | Medium   | 8 units      | Round Robin | Q3         |
| Q3    | Low      | Unlimited    | FCFS        | N/A        |
```
---

##  Scheduling Algorithm

1. **Arrival**: New process enters Q1 of the lesser-loaded CPU.
2. **Execution**: Process runs within its time quantum.
3. **Completion**: Process terminates → collect metrics.
4. **Demotion**: If not done, it's moved to the next queue.
5. **CPU Help**: Idle CPU contributes one unit to the active CPU’s task.

### Load Balancing

* Compares **total remaining burst** of CPUs.
* Routes new processes to the one with lesser workload.

### Inter-Processor Help

* Idle CPU helps the other for one unit
* Contributions logged in metrics + Gantt

---

##  Build & Run Instructions

###  Prerequisites

* C++17 compiler (GCC/Clang)
* Make or CMake (optional)

###  Build Options

**Using Make (Recommended)**

```bash
git clone <https://github.com/Harsha-Medicharla/Enhanced-MLFQ_Scheduler.git>
cd Enhanced-MLFQ_Scheduler
make     # build
make run # run
make clean # clean artifacts
```

**Using CMake**

```bash
mkdir build && cd build
cmake ..
make
```

**Manual**

```bash
g++ -std=c++17 -Wall -Wextra -O2 -o mlfq MLFQ_2PROCESSOR.cpp
```

---

##  Usage

```bash
./mlfq
```

Choose input method:

1. Terminal
2. File (`input.txt`)

---

##  Input Format

**Terminal**

```
Enter the number of processes: 3
Enter arrival time and burst time for process 1: 0 10
Enter arrival time and burst time for process 2: 2 8
Enter arrival time and burst time for process 3: 4 6
```

**File (input.txt)**

```
3
0 10
2 8
4 6
```

---

##  Output Format

###  Process Summary

```
PID   AT      BT      WT      TAT       CT      RT      Done By     Helped By   
================================================================================
1     0       10      5       13        13      0       CPU1        CPU2        
2     2       8       0       8         10      0       CPU2        None        
3     4       6       4       10        14      1       CPU1        None        
================================================================================
```

### Performance Metrics

```
Average Waiting Time:     3.00 units
Average Turnaround Time:  10.33 units
Average Response Time:    0.33 units
Total Completion Time:    14 units
CPU Utilization:          85.71%

Queue Work Distribution:
High Priority Queue:      15 units
Medium Priority Queue:    9 units
Low Priority Queue:       0 units
Total Work Done:          24 units
```

###  Gantt Chart

```
CPU1 |P1  P1  P1  P1  P1  P3  P3  P3  P3  P3  P1  P1  P1  P3  |
      0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  

CPU2 |H1  H1  P2  P2  P2  P2  P2  P2  P2  P2        --- |
      0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  
```

Legend:

* P#: Process execution
* H#: Help from another CPU
* \---: Idle

### Detailed Execution Timeline

```
Time      CPU   Process   Queue   Action              Duration
-----------------------------------------------------------------
0-5       CPU1  P1        Q1      Executing in Q1     5 unit(s)
0-1       CPU2  P1        HELP    Helping other CPU   1 unit(s)
1-2       CPU2  P1        HELP    Helping other CPU   1 unit(s)
2-7       CPU2  P2        Q1      Executing in Q1     5 unit(s)
5-10      CPU1  P3        Q1      Executing in Q1     5 unit(s)
7-10      CPU2  P2        Q2      Executing in Q2     3 unit(s)
10-13     CPU1  P1        Q2      Executing in Q2     3 unit(s)
13-14     CPU1  P3        Q2      Executing in Q2     1 unit(s)
13-14     CPU2  IDLE      -       Processor idle      1 unit(s)
```

###  Execution Summary

CPU1 - Process Execution: 14 units, Help Time: 0 units, Idle Time: 0 units
CPU2 - Process Execution: 8 units, Help Time: 2 units, Idle Time: 1 units

---

##  Project Structure

```
Enhanced-MLFQ_Scheduler/
├── MLFQ_2PROCESSOR.h
├── MLFQ_2PROCESSOR.cpp
├── input.txt / output.txt
├── Makefile / CMakeLists.txt
├── .vscode/
└── README.md
```

---

##  **Contributing**

Contributions to this project are welcome. If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---
