#include "MLFQ_2PROCESSOR.h"

// Static member definitions
ofstream MLFQ_2PROCESSOR::fw;
int MLFQ_2PROCESSOR::highQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::mediumQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::lowQueueJobWorkDone = 0;
map<MLFQ_2PROCESSOR::Process*, int> MLFQ_2PROCESSOR::processContributionMap;
vector<MLFQ_2PROCESSOR::GanttEntry> MLFQ_2PROCESSOR::ganttChart;

// Constructor implementations
MLFQ_2PROCESSOR::Process::Process() : pid(0), arrivalTime(0), burstTime(0), remainingBurstTime(0), 
                                     waitingTime(0), turnaroundTime(0), completionTime(0), 
                                     completedBy(0), contributedBy(0), responseTime(-1), hasStarted(false) {}

MLFQ_2PROCESSOR::Process::Process(int p, int at, int bt) : pid(p), arrivalTime(at), burstTime(bt), 
                                                          remainingBurstTime(bt), waitingTime(0), 
                                                          turnaroundTime(0), completionTime(0), 
                                                          completedBy(0), contributedBy(0), 
                                                          responseTime(-1), hasStarted(false) {}



MLFQ_2PROCESSOR::Processor::Processor(int i, bool free, int load, int time) : 
    id(i), isFree(free), totalLoad(load), currentTime(time) {}

void MLFQ_2PROCESSOR::takeInput(int n, vector<Process>& processes) {
    for (int i = 0; i < n; i++) {
        cout << "Enter arrival time and burst time for process " << (i + 1) << ": ";
        int arrivalTime, burstTime;
        cin >> arrivalTime >> burstTime;
        processes[i] = Process(i + 1, arrivalTime, burstTime);
    }

    sort(processes.begin(), processes.end(), 
         [](const Process& a, const Process& b) {
             return a.arrivalTime < b.arrivalTime;
         });
}

int MLFQ_2PROCESSOR::getTotalRemainingTime(Processor& processor) {
    int remaining = 0;
    
    queue<Process*> tempQ1 = processor.q1;
    while (!tempQ1.empty()) {
        remaining += tempQ1.front()->remainingBurstTime;
        tempQ1.pop();
    }
    
    queue<Process*> tempQ2 = processor.q2;
    while (!tempQ2.empty()) {
        remaining += tempQ2.front()->remainingBurstTime;
        tempQ2.pop();
    }
    
    queue<Process*> tempQ3 = processor.q3;
    while (!tempQ3.empty()) {
        remaining += tempQ3.front()->remainingBurstTime;
        tempQ3.pop();
    }
    
    return remaining;
}

void MLFQ_2PROCESSOR::contributeToOtherProcessor(Processor& helper, Processor& target, int helperId) {
    // Helper processor contributes 1 time unit to the target's current process
    int startTime = helper.currentTime;
    if (!target.q1.empty()) {
        Process* p = target.q1.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            highQueueJobWorkDone++;
            processContributionMap[p]++;
            p->contributedBy=helperId;

            // Add to Gantt chart
            ganttChart.push_back(GanttEntry(helperId, p->pid, startTime, startTime + 1, "HELP"));

        }
    } else if (!target.q2.empty()) {
        Process* p = target.q2.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            mediumQueueJobWorkDone++;
            processContributionMap[p]++;
            p->contributedBy=helperId;

            // Add to Gantt chart
            ganttChart.push_back(GanttEntry(helperId, p->pid, startTime, startTime + 1, "HELP"));

        }
    } else if (!target.q3.empty()) {
        Process* p = target.q3.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            lowQueueJobWorkDone++;
            processContributionMap[p]++;
           p->contributedBy=helperId;

           // Add to Gantt chart
            ganttChart.push_back(GanttEntry(helperId, p->pid, startTime, startTime + 1, "HELP"));

        }
    }

    else {
        // No process to help, processor is idle
        ganttChart.push_back(GanttEntry(helperId, 0, startTime, startTime + 1, "IDLE"));
    }
}

void MLFQ_2PROCESSOR::executeProcess(Process* process, Processor& processor, 
                                   int timeQuantum, int& currentTime) {
    if (!process->hasStarted) {
        process->responseTime = currentTime - process->arrivalTime;
        process->hasStarted = true;
    }
    
    int startTime = currentTime;
    int executionTime = min(timeQuantum, process->remainingBurstTime);
    process->remainingBurstTime -= executionTime;
    currentTime += executionTime;
    
    // Track work done by queue type

     // Determine queue type for Gantt chart
    string queueType;

    if (timeQuantum == 5) {
        queueType = "Q1";
        highQueueJobWorkDone += executionTime;
    } else if (timeQuantum == 8) {
        queueType = "Q2";
        mediumQueueJobWorkDone += executionTime;
    } else {
        queueType = "Q3";
        lowQueueJobWorkDone += executionTime;
    }

    // Add to Gantt chart
    ganttChart.push_back(GanttEntry(processor.id, process->pid, startTime, currentTime, queueType));

}

void MLFQ_2PROCESSOR::run(vector<Process>& processes, int n) {
    if (n == 0) {
        cout << "Program doesn't run with 0 processes" << endl;
        return;
    }

    Processor processor1(1, true, 0, 0);
    Processor processor2(2, true, 0, 0);
    
    int completed = 0;
    int processIndex = 0;
    
    // Handle initial time setup
    if (!processes.empty() && processes[0].arrivalTime > 0) {
        processor1.currentTime = processes[0].arrivalTime;
        processor2.currentTime = processes[0].arrivalTime;

        // Add idle time to Gantt chart if needed
        if (processes[0].arrivalTime > 0) {
            ganttChart.push_back(GanttEntry(1, 0, 0, processes[0].arrivalTime, "IDLE"));
            ganttChart.push_back(GanttEntry(2, 0, 0, processes[0].arrivalTime, "IDLE"));
        }
    }
    
    // Main simulation loop
    while (completed < n) {
        int currentTime = min(processor1.currentTime, processor2.currentTime);
        
        // Add arrived processes to appropriate processor queues
        while (processIndex < n && processes[processIndex].arrivalTime <= currentTime) {
            Process* process = &processes[processIndex];
            
            // Load balancing: assign to processor with less total remaining work
            int remaining1 = getTotalRemainingTime(processor1);
            int remaining2 = getTotalRemainingTime(processor2);
            
            if (remaining1 <= remaining2) {
                processor1.q1.push(process);
            } else {
                processor2.q1.push(process);
            }
            processIndex++;
        }
        
        // Execute on processor 1
        if (currentTime == processor1.currentTime) {
            bool executed = false;
            
            // Priority: Q1 -> Q2 -> Q3
            if (!processor1.q1.empty()) {
                Process* p = processor1.q1.front();
                processor1.q1.pop();
                
                executeProcess(p, processor1, 5, processor1.currentTime);
                
                if (p->remainingBurstTime == 0) {
                    // Process completed
                    p->completionTime = processor1.currentTime;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    
                    // Add contribution time to waiting time
                    if (processContributionMap.find(p) != processContributionMap.end()) {
                        p->waitingTime += processContributionMap[p];
                    }
                    
                    p->completedBy = 1;
                    completed++;
                } else {
                    // Move to next queue
                    processor1.q2.push(p);
                }
                executed = true;
                
            } else if (!processor1.q2.empty()) {
                Process* p = processor1.q2.front();
                processor1.q2.pop();
                
                executeProcess(p, processor1, 8, processor1.currentTime);
                
                if (p->remainingBurstTime == 0) {
                    // Process completed
                    p->completionTime = processor1.currentTime;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    
                    // Add contribution time to waiting time
                    if (processContributionMap.find(p) != processContributionMap.end()) {
                        p->waitingTime += processContributionMap[p];
                    }
                    
                    p->completedBy = 1;
                    completed++;
                } else {
                    // Move to next queue
                    processor1.q3.push(p);
                }
                executed = true;
                
            } else if (!processor1.q3.empty()) {
                Process* p = processor1.q3.front();
                processor1.q3.pop();
                
                // FCFS: execute entire remaining burst time
                executeProcess(p, processor1, p->remainingBurstTime, processor1.currentTime);
                
                p->completionTime = processor1.currentTime;
                p->turnaroundTime = p->completionTime - p->arrivalTime;
                p->waitingTime = p->turnaroundTime - p->burstTime;
                
                // Add contribution time to waiting time
                if (processContributionMap.find(p) != processContributionMap.end()) {
                    p->waitingTime += processContributionMap[p];
                }
                
                p->completedBy = 1;
                completed++;
                executed = true;
            }
            
            if (!executed) {
                // No process to execute, try to help processor 2
                if (!processor2.q1.empty() || !processor2.q2.empty() || !processor2.q3.empty()) {
                    contributeToOtherProcessor(processor1, processor2, 1);
                }
                else {
                    // Add idle time to Gantt chart
                    ganttChart.push_back(GanttEntry(1, 0, processor1.currentTime, processor1.currentTime + 1, "IDLE"));
                }
                processor1.currentTime++;
            }
        }
        
        // Execute on processor 2 (similar logic)
        if (currentTime == processor2.currentTime) {
            bool executed = false;
            
            // Priority: Q1 -> Q2 -> Q3
            if (!processor2.q1.empty()) {
                Process* p = processor2.q1.front();
                processor2.q1.pop();
                
                executeProcess(p, processor2, 5, processor2.currentTime);
                
                if (p->remainingBurstTime == 0) {
                    // Process completed
                    p->completionTime = processor2.currentTime;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    
                    // Add contribution time to waiting time
                    if (processContributionMap.find(p) != processContributionMap.end()) {
                        p->waitingTime += processContributionMap[p];
                    }
                    
                    p->completedBy = 2;
                    completed++;
                } else {
                    // Move to next queue
                    processor2.q2.push(p);
                }
                executed = true;
                
            } else if (!processor2.q2.empty()) {
                Process* p = processor2.q2.front();
                processor2.q2.pop();
                
                executeProcess(p, processor2, 8, processor2.currentTime);
                
                if (p->remainingBurstTime == 0) {
                    // Process completed
                    p->completionTime = processor2.currentTime;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    
                    // Add contribution time to waiting time
                    if (processContributionMap.find(p) != processContributionMap.end()) {
                        p->waitingTime += processContributionMap[p];
                    }
                    
                    p->completedBy = 2;
                    completed++;
                } else {
                    // Move to next queue
                    processor2.q3.push(p);
                }
                executed = true;
                
            } else if (!processor2.q3.empty()) {
                Process* p = processor2.q3.front();
                processor2.q3.pop();
                
                // FCFS: execute entire remaining burst time
                executeProcess(p, processor2, p->remainingBurstTime, processor2.currentTime);
                
                p->completionTime = processor2.currentTime;
                p->turnaroundTime = p->completionTime - p->arrivalTime;
                p->waitingTime = p->turnaroundTime - p->burstTime;
                
                // Add contribution time to waiting time
                if (processContributionMap.find(p) != processContributionMap.end()) {
                    p->waitingTime += processContributionMap[p];
                }
                
                p->completedBy = 2;
                completed++;
                executed = true;
            }
            
            if (!executed) {
                // No process to execute, try to help processor 1
                if (!processor1.q1.empty() || !processor1.q2.empty() || !processor1.q3.empty()) {
                    contributeToOtherProcessor(processor2, processor1, 2);
                }
                else {
                    // Add idle time to Gantt chart
                    ganttChart.push_back(GanttEntry(2, 0, processor2.currentTime, processor2.currentTime + 1, "IDLE"));
                }
                processor2.currentTime++;
            }
        }
        
        // Safety check to prevent infinite loop
        if (processIndex >= n && completed == 0 && 
            processor1.q1.empty() && processor1.q2.empty() && processor1.q3.empty() &&
            processor2.q1.empty() && processor2.q2.empty() && processor2.q3.empty()) {
            break;
        }
    }
}


void MLFQ_2PROCESSOR::printGanttChart() {
    fw << "\n" << string(80, '=') << endl;
    fw << "GANTT CHART" << endl;
    fw << string(80, '=') << endl;
    
    // Sort Gantt chart entries by processor ID and start time
    sort(ganttChart.begin(), ganttChart.end(), 
         [](const GanttEntry& a, const GanttEntry& b) {
             if (a.processorId != b.processorId) {
                 return a.processorId < b.processorId;
             }
             return a.startTime < b.startTime;
         });
    
    // Find the maximum completion time
    int maxTime = 0;
    for (const auto& entry : ganttChart) {
        maxTime = max(maxTime, entry.endTime);
    }
    
    // Print CPU1 timeline
    fw << "\nCPU1 |";
    int currentPos = 0;
    for (const auto& entry : ganttChart) {
        if (entry.processorId == 1) {
            // Fill gaps if any
            while (currentPos < entry.startTime) {
                fw << "  ";
                currentPos++;
            }
            
            // Print the process execution
            for (int t = entry.startTime; t < entry.endTime; t++) {
                if (entry.processId == 0) {
                    fw << setw(4) << "---";  // Idle
                } else {
                    if (entry.queueType == "HELP") {
                        fw << setw(4) << ("H" + to_string(entry.processId));
                    } else {
                       fw << setw(4) << ("P" + to_string(entry.processId));
                    }
                }
            }
            currentPos = entry.endTime;
        }
    }
    fw << "|" << endl;
    
    // Print CPU1 time markers
    fw << "      ";  // Align with "CPUx |"
    for (int i = 0; i <= maxTime; i++) {
        fw << setw(4) << i;
    }

    fw << endl;
    
    // Print CPU2 timeline
    fw << "\nCPU2 |";
    currentPos = 0;
    for (const auto& entry : ganttChart) {
        if (entry.processorId == 2) {
            // Fill gaps if any
            while (currentPos < entry.startTime) {
                fw << "  ";
                currentPos++;
            }
            
            // Print the process execution
            for (int t = entry.startTime; t < entry.endTime; t++) {
                if (entry.processId == 0) {
                     fw << setw(4) << "---"; // Idle
                } else {
                    if (entry.queueType == "HELP") {
                        fw << setw(4) << ("H" + to_string(entry.processId));
                    } else {
                       fw << setw(4) << ("P" + to_string(entry.processId));
                    }
                }
            }
            currentPos = entry.endTime;
        }
    }
    fw << "|" << endl;
    
    // Print CPU2 time markers
    fw << "      ";  // Align with "CPUx |"
    for (int i = 0; i <= maxTime; i++) {
        fw << setw(4) << i;
    }

    fw << endl;
    
    // Print legend with enhanced information
    fw << "\nLegend:" << endl;
    fw << "P1, P2, P3... = Process execution on respective processor" << endl;
    fw << "H1, H2, H3... = Help from other processor" << endl;
    fw << "--             = Processor idle" << endl;
    fw << "\nQueue Information:" << endl;
    fw << "Q1 = High Priority (Time Quantum: 5 units)" << endl;
    fw << "Q2 = Medium Priority (Time Quantum: 8 units)" << endl;
    fw << "Q3 = Low Priority (FCFS - no time quantum)" << endl;
    
    // Print detailed execution timeline with better formatting
    fw << "\nDetailed Execution Timeline:" << endl;
    fw << left << setw(10) << "Time" << setw(6) << "CPU" << setw(10) << "Process" 
       << setw(8) << "Queue" << setw(20) << "Action" << "Duration" << endl;
    fw << string(65, '-') << endl;
    
    // Sort all entries by time for detailed timeline
    vector<GanttEntry> sortedEntries = ganttChart;
    sort(sortedEntries.begin(), sortedEntries.end(), 
         [](const GanttEntry& a, const GanttEntry& b) {
             if (a.startTime != b.startTime) {
                 return a.startTime < b.startTime;
             }
             return a.processorId < b.processorId;
         });
    
    for (const auto& entry : sortedEntries) {
        string timeRange = to_string(entry.startTime) + "-" + to_string(entry.endTime);
        int duration = entry.endTime - entry.startTime;
        
        fw << left << setw(10) << timeRange
           << setw(6) << ("CPU" + to_string(entry.processorId));
        
        if (entry.processId == 0) {
            fw << setw(10) << "IDLE" << setw(8) << "-" 
               << setw(20) << "Processor idle" << duration << " unit(s)" << endl;
        } else {
            fw << setw(10) << ("P" + to_string(entry.processId))
               << setw(8) << entry.queueType;
            
            if (entry.queueType == "HELP") {
                fw << setw(20) << "Helping other CPU" << duration << " unit(s)" << endl;
            } else {
                fw << setw(20) << ("Executing in " + entry.queueType) << duration << " unit(s)" << endl;
            }
        }
    }
    
    // Add summary statistics
    fw << "\nExecution Summary:" << endl;
    fw << string(30, '-') << endl;
    
    // Count process executions per CPU
    map<int, int> cpuProcessCount;
    map<int, int> cpuHelpCount;
    map<int, int> cpuIdleTime;
    
    for (const auto& entry : ganttChart) {
        int duration = entry.endTime - entry.startTime;
        if (entry.processId == 0) {
            cpuIdleTime[entry.processorId] += duration;
        } else if (entry.queueType == "HELP") {
            cpuHelpCount[entry.processorId] += duration;
        } else {
            cpuProcessCount[entry.processorId] += duration;
        }
    }
    
    fw << "CPU1 - Process Execution: " << cpuProcessCount[1] << " units, "
       << "Help Time: " << cpuHelpCount[1] << " units, "
       << "Idle Time: " << cpuIdleTime[1] << " units" << endl;
    fw << "CPU2 - Process Execution: " << cpuProcessCount[2] << " units, "
       << "Help Time: " << cpuHelpCount[2] << " units, "
       << "Idle Time: " << cpuIdleTime[2] << " units" << endl;
}


void MLFQ_2PROCESSOR::printFinalStates(const vector<Process>& processes) {
    fw << left << setw(6) << "PID" 
       << setw(8) << "AT" 
       << setw(8) << "BT" 
       << setw(8) << "WT" 
       << setw(10) << "TAT" 
       << setw(8) << "CT" 
       << setw(8) << "RT"
       << setw(12) << "Done By" 
       << setw(12) << "Helped By" << endl;
    
    fw << string(80, '=') << endl;
    

    
    for (const auto& process : processes) {

                string helpedBy = (process.contributedBy != 0) ? 
                ("CPU" + to_string(process.contributedBy)) : "None";


        
        fw << left << setw(6) << process.pid
           << setw(8) << process.arrivalTime
           << setw(8) << process.burstTime
           << setw(8) << process.waitingTime
           << setw(10) << process.turnaroundTime
           << setw(8) << process.completionTime
           << setw(8) << process.responseTime
           << setw(12) << ("CPU" + to_string(process.completedBy))
           << setw(12) << helpedBy << endl;
    }
    
    fw << string(80, '=') << endl;
}

void MLFQ_2PROCESSOR::calculateAndPrintMetrics(const vector<Process>& processes) {
    if (processes.empty()) return;
    
    double totalWaitingTime = 0;
    double totalTurnaroundTime = 0;
    double totalResponseTime = 0;
    int maxCompletionTime = 0;
    
    for (const auto& process : processes) {
        totalWaitingTime += process.waitingTime;
        totalTurnaroundTime += process.turnaroundTime;
        totalResponseTime += process.responseTime;
        
        if (process.completionTime > maxCompletionTime) {
            maxCompletionTime = process.completionTime;
        }
    }
    
    int n = processes.size();
    double avgWaitingTime = totalWaitingTime / n;
    double avgTurnaroundTime = totalTurnaroundTime / n;
    double avgResponseTime = totalResponseTime / n;
    double cpuUtilization = ((double)(highQueueJobWorkDone + mediumQueueJobWorkDone + lowQueueJobWorkDone) 
                           / (maxCompletionTime * 2)) * 100; // 2 processors
    
    fw << "\n" << string(50, '=') << endl;
    fw << "PERFORMANCE METRICS" << endl;
    fw << string(50, '=') << endl;
    
    fw << fixed << setprecision(2);
    fw << "Average Waiting Time:     " << avgWaitingTime << " units" << endl;
    fw << "Average Turnaround Time:  " << avgTurnaroundTime << " units" << endl;
    fw << "Average Response Time:    " << avgResponseTime << " units" << endl;
    fw << "Total Completion Time:    " << maxCompletionTime << " units" << endl;
    fw << "CPU Utilization:          " << cpuUtilization << "%" << endl;
    
    fw << "\nQueue Work Distribution:" << endl;
    fw << "High Priority Queue:      " << highQueueJobWorkDone << " units" << endl;
    fw << "Medium Priority Queue:    " << mediumQueueJobWorkDone << " units" << endl;
    fw << "Low Priority Queue:       " << lowQueueJobWorkDone << " units" << endl;
    
    int totalWork = highQueueJobWorkDone + mediumQueueJobWorkDone + lowQueueJobWorkDone;
    fw << "Total Work Done:          " << totalWork << " units" << endl;
}

vector<MLFQ_2PROCESSOR::Process> MLFQ_2PROCESSOR::takeTerminalInput() {
    cout << "Enter the number of processes: ";
    int n;
    cin >> n;
    
    if (n <= 0) {
        cout << "Invalid number of processes!" << endl;
        return vector<Process>();
    }
    
    vector<Process> processes(n);
    
    for (int i = 0; i < n; i++) {
        cout << "Enter arrival time and burst time for process " << (i + 1) << ": ";
        int arrivalTime, burstTime;
        cin >> arrivalTime >> burstTime;
        
        if (arrivalTime < 0 || burstTime <= 0) {
            cout << "Invalid input! Arrival time must be >= 0 and burst time must be > 0" << endl;
            i--; // Retry this process
            continue;
        }
        
        processes[i] = Process(i + 1, arrivalTime, burstTime);
    }
    
    sort(processes.begin(), processes.end(), 
         [](const Process& a, const Process& b) {
             return a.arrivalTime < b.arrivalTime;
         });
    
    return processes;
}

vector<MLFQ_2PROCESSOR::Process> MLFQ_2PROCESSOR::takeFileInput() {
    try {
        ifstream file("input.txt");
        if (!file.is_open()) {
            cout << "Error: Could not open input.txt file!" << endl;
            return vector<Process>();
        }
        
        int n;
        if (!(file >> n) || n <= 0) {
            cout << "Error: Invalid number of processes in file!" << endl;
            file.close();
            return vector<Process>();
        }
        
        vector<Process> processes(n);
        
        for (int i = 0; i < n; i++) {
            int arrivalTime, burstTime;
            if (!(file >> arrivalTime >> burstTime)) {
                cout << "Error: Could not read process " << (i + 1) << " data from file!" << endl;
                file.close();
                return vector<Process>();
            }
            
            if (arrivalTime < 0 || burstTime <= 0) {
                cout << "Error: Invalid data for process " << (i + 1) << "!" << endl;
                file.close();
                return vector<Process>();
            }
            
            processes[i] = Process(i + 1, arrivalTime, burstTime);
        }
        
        file.close();
        
        sort(processes.begin(), processes.end(), 
             [](const Process& a, const Process& b) {
                 return a.arrivalTime < b.arrivalTime;
             });
        
        return processes;
    } catch (const exception& e) {
        cout << "Error reading file: " << e.what() << endl;
        return vector<Process>();
    }
}

void MLFQ_2PROCESSOR::resetStaticVariables() {
    highQueueJobWorkDone = 0;
    mediumQueueJobWorkDone = 0;
    lowQueueJobWorkDone = 0;
    processContributionMap.clear();
    ganttChart.clear();
}

void MLFQ_2PROCESSOR::execute() {
    cout << "===========================================" << endl;
    cout << "Multi-Level Feedback Queue Scheduler" << endl;
    cout << "     (Dual Processor System)" << endl;
    cout << "===========================================" << endl;
    cout << "\nQueue Configuration:" << endl;
    cout << "Q1 (High Priority):   Time Quantum = 5 units" << endl;
    cout << "Q2 (Medium Priority): Time Quantum = 8 units" << endl;
    cout << "Q3 (Low Priority):    FCFS (no time quantum)" << endl;
    cout << "\nChoose input method:" << endl;
    cout << "1) Terminal input" << endl;
    cout << "2) File input (input.txt)" << endl;
    cout << "Enter your choice (1 or 2): ";
    
    int choice;
    cin >> choice;
    
    vector<Process> processes;
    
    if (choice == 2) {
        processes = takeFileInput();
        if (!processes.empty()) {
            cout << "\nSuccessfully loaded " << processes.size() << " processes from input.txt" << endl;
        }
    } else if (choice == 1) {
        processes = takeTerminalInput();
    } else {
        cout << "Invalid choice! Using terminal input by default." << endl;
        processes = takeTerminalInput();
    }
    
    if (processes.empty()) {
        cout << "No valid processes to execute!" << endl;
        return;
    }
    
    cout << "\nProcess Information:" << endl;
    cout << "PID\tArrival\tBurst" << endl;
    cout << "---\t-------\t-----" << endl;
    for (const auto& p : processes) {
        cout << p.pid << "\t" << p.arrivalTime << "\t" << p.burstTime << endl;
    }
    
    cout << "\nRunning MLFQ scheduler..." << endl;
    
    // Reset static variables
    resetStaticVariables();
    
    // Run the scheduler
    run(processes, processes.size());
    
    // Write output to file
    fw.open("output.txt");
    if (!fw.is_open()) {
        cout << "Error: Could not create output.txt file!" << endl;
        return;
    }
    
    fw << "MULTI-LEVEL FEEDBACK QUEUE SCHEDULER RESULTS" << endl;
    fw << "Dual Processor System" << endl;
    fw << "Generated on: " << __DATE__ << " " << __TIME__ << endl;
    fw << string(50, '=') << endl << endl;
    
    printFinalStates(processes);
    calculateAndPrintMetrics(processes);
    printGanttChart();
    
    fw.close();
    
    cout << "\n===========================================" << endl;
    cout << "Execution completed successfully!" << endl;
    cout << "Results have been saved to 'output.txt'" << endl;
    cout << "===========================================" << endl;
}

int main() {
    MLFQ_2PROCESSOR::execute();
    return 0;
}