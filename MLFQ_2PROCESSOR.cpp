#include "MLFQ_2PROCESSOR.h"

// Static member definitions
ofstream MLFQ_2PROCESSOR::fw;
int MLFQ_2PROCESSOR::highQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::mediumQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::lowQueueJobWorkDone = 0;
map<MLFQ_2PROCESSOR::Process*, int> MLFQ_2PROCESSOR::processContributionMap;

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
    if (!target.q1.empty()) {
        Process* p = target.q1.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            highQueueJobWorkDone++;
            processContributionMap[p]++;
            p->contributedBy = helperId;
        }
    } else if (!target.q2.empty()) {
        Process* p = target.q2.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            mediumQueueJobWorkDone++;
            processContributionMap[p]++;
            p->contributedBy = helperId;
        }
    } else if (!target.q3.empty()) {
        Process* p = target.q3.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            lowQueueJobWorkDone++;
            processContributionMap[p]++;
            p->contributedBy = helperId;
        }
    }
}

void MLFQ_2PROCESSOR::executeProcess(Process* process, Processor& processor, 
                                   int timeQuantum, int& currentTime) {
    if (!process->hasStarted) {
        process->responseTime = currentTime - process->arrivalTime;
        process->hasStarted = true;
    }
    
    int executionTime = min(timeQuantum, process->remainingBurstTime);
    process->remainingBurstTime -= executionTime;
    currentTime += executionTime;
    
    // Track work done by queue type
    if (timeQuantum == 5) {
        highQueueJobWorkDone += executionTime;
    } else if (timeQuantum == 8) {
        mediumQueueJobWorkDone += executionTime;
    } else {
        lowQueueJobWorkDone += executionTime;
    }
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