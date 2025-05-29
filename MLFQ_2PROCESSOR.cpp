#include "MLFQ_2PROCESSOR.h"

// Static member definitions
ofstream MLFQ_2PROCESSOR::fw;
int MLFQ_2PROCESSOR::highQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::mediumQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::lowQueueJobWorkDone = 0;
map<MLFQ_2PROCESSOR::Process*, int> MLFQ_2PROCESSOR::processMap;

// Constructor implementations
MLFQ_2PROCESSOR::Process::Process() : pid(0), arrivalTime(0), burstTime(0), remainingBurstTime(0), 
                                     waitingTime(0), turnaroundTime(0), completionTime(0), 
                                     completedBy(0), contributedBy(0) {}

MLFQ_2PROCESSOR::Process::Process(int p, int at, int bt) : pid(p), arrivalTime(at), burstTime(bt), 
                                                          remainingBurstTime(bt), waitingTime(0), 
                                                          turnaroundTime(0), completionTime(0), 
                                                          completedBy(0), contributedBy(0) {}

MLFQ_2PROCESSOR::Processor::Processor(int i, bool free, int load) : id(i), isFree(free), totalLoad(load) {}

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

int MLFQ_2PROCESSOR::remainingTime(Processor& processor) {
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

void MLFQ_2PROCESSOR::contribute(Processor& check, int id) {
    if (!check.q1.empty()) {
        Process* p = check.q1.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            highQueueJobWorkDone++;
            processMap[p]++;
            p->contributedBy = id;
        }
    } else if (!check.q2.empty()) {
        Process* p = check.q2.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            mediumQueueJobWorkDone++;
            processMap[p]++;
            p->contributedBy = id;
        }
    } else if (!check.q3.empty()) {
        Process* p = check.q3.front();
        if (p->remainingBurstTime > 1) {
            p->remainingBurstTime--;
            lowQueueJobWorkDone++;
            processMap[p]++;
            p->contributedBy = id;
        }
    }
}

void MLFQ_2PROCESSOR::run(vector<Process>& processes, int n) {
    if (n == 0) {
        cout << "Program doesn't run with 0 processes" << endl;
        return;
    }

    Processor processor1(1, true, 0);
    Processor processor2(2, true, 0);

    int time1 = 0;
    int time2 = 0;
    int completed = 0;
    int time = 0;
    queue<Process*> mainQueue;

    // Initialize main queue with processes
    for (auto& process : processes) {
        mainQueue.push(&process);
    }

    // Handle initial time setup
    if (!mainQueue.empty() && mainQueue.front()->arrivalTime > 0) {
        time1 = mainQueue.front()->arrivalTime;
        time2 = time1;
        time = time1;
    }

    // Main simulation loop
    while (completed < n) {
        time = min(time1, time2);
        
        // Add arrived processes to appropriate queues
        while (!mainQueue.empty() && mainQueue.front()->arrivalTime <= time) {
            Process* process = mainQueue.front();
            mainQueue.pop();
            if (remainingTime(processor1) <= remainingTime(processor2)) {
                processor1.q1.push(process);
            } else {
                processor2.q1.push(process);
            }
        }

        // Processor 1 execution
        if (time == time1) {
            if (!processor1.q1.empty()) {
                Process* p = processor1.q1.front();
                processor1.q1.pop();
                
                if (p->remainingBurstTime <= 5) {
                    // Complete in Q1
                    time1 += p->remainingBurstTime;
                    highQueueJobWorkDone += p->remainingBurstTime;
                    p->completionTime = time1;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    if (processMap.find(p) != processMap.end()) {
                        p->waitingTime += processMap[p];
                    }
                    p->completedBy = 1;
                    completed++;
                } else {
                    // Move to Q2
                    p->remainingBurstTime -= 5;
                    time1 += 5;
                    highQueueJobWorkDone += 5;
                    processor1.q2.push(p);
                }
            } else if (!processor1.q2.empty()) {
                Process* p = processor1.q2.front();
                processor1.q2.pop();
                
                if (p->remainingBurstTime <= 8) {
                    // Complete in Q2
                    time1 += p->remainingBurstTime;
                    mediumQueueJobWorkDone += p->remainingBurstTime;
                    p->completionTime = time1;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    if (processMap.find(p) != processMap.end()) {
                        p->waitingTime += processMap[p];
                    }
                    p->completedBy = 1;
                    completed++;
                } else {
                    // Move to Q3
                    p->remainingBurstTime -= 8;
                    time1 += 8;
                    mediumQueueJobWorkDone += 8;
                    processor1.q3.push(p);
                }
            } else if (!processor1.q3.empty()) {
                Process* p = processor1.q3.front();
                processor1.q3.pop();
                
                // Execute entire remaining burst time in Q3 (FCFS)
                time1 += p->remainingBurstTime;
                lowQueueJobWorkDone += p->remainingBurstTime;
                p->completionTime = time1;
                p->turnaroundTime = p->completionTime - p->arrivalTime;
                p->waitingTime = p->turnaroundTime - p->burstTime;
                if (processMap.find(p) != processMap.end()) {
                    p->waitingTime += processMap[p];
                }
                p->completedBy = 1;
                completed++;
            } else {
                // No process to execute, help other processor or idle
                if (!processor2.q1.empty() || !processor2.q2.empty() || !processor2.q3.empty()) {
                    contribute(processor2, 1);
                }
                time1++;
            }
        }

        // Processor 2 execution (similar logic)
        if (time == time2) {
            if (!processor2.q1.empty()) {
                Process* p = processor2.q1.front();
                processor2.q1.pop();
                
                if (p->remainingBurstTime <= 5) {
                    // Complete in Q1
                    time2 += p->remainingBurstTime;
                    highQueueJobWorkDone += p->remainingBurstTime;
                    p->completionTime = time2;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    if (processMap.find(p) != processMap.end()) {
                        p->waitingTime += processMap[p];
                    }
                    p->completedBy = 2;
                    completed++;
                } else {
                    // Move to Q2
                    p->remainingBurstTime -= 5;
                    time2 += 5;
                    highQueueJobWorkDone += 5;
                    processor2.q2.push(p);
                }
            } else if (!processor2.q2.empty()) {
                Process* p = processor2.q2.front();
                processor2.q2.pop();
                
                if (p->remainingBurstTime <= 8) {
                    // Complete in Q2
                    time2 += p->remainingBurstTime;
                    mediumQueueJobWorkDone += p->remainingBurstTime;
                    p->completionTime = time2;
                    p->turnaroundTime = p->completionTime - p->arrivalTime;
                    p->waitingTime = p->turnaroundTime - p->burstTime;
                    if (processMap.find(p) != processMap.end()) {
                        p->waitingTime += processMap[p];
                    }
                    p->completedBy = 2;
                    completed++;
                } else {
                    // Move to Q3
                    p->remainingBurstTime -= 8;
                    time2 += 8;
                    mediumQueueJobWorkDone += 8;
                    processor2.q3.push(p);
                }
            } else if (!processor2.q3.empty()) {
                Process* p = processor2.q3.front();
                processor2.q3.pop();
                
                // Execute entire remaining burst time in Q3 (FCFS)
                time2 += p->remainingBurstTime;
                lowQueueJobWorkDone += p->remainingBurstTime;
                p->completionTime = time2;
                p->turnaroundTime = p->completionTime - p->arrivalTime;
                p->waitingTime = p->turnaroundTime - p->burstTime;
                if (processMap.find(p) != processMap.end()) {
                    p->waitingTime += processMap[p];
                }
                p->completedBy = 2;
                completed++;
            } else {
                // No process to execute, help other processor or idle
                if (!processor1.q1.empty() || !processor1.q2.empty() || !processor1.q3.empty()) {
                    contribute(processor1, 2);
                }
                time2++;
            }
        }
        
        // Safety check to prevent infinite loop
        if (completed == 0 && mainQueue.empty() && 
            processor1.q1.empty() && processor1.q2.empty() && processor1.q3.empty() &&
            processor2.q1.empty() && processor2.q2.empty() && processor2.q3.empty()) {
            break;
        }
    }
}

void MLFQ_2PROCESSOR::printFinalStates(const vector<Process>& processes) {
    fw << "PID\t\tAT\t\tBT\t\tWT\t\tTurnT\t\tCompT\t\tDone By\t\tHelped By\n";
    fw << "================================================================================\n";
    for (const auto& process : processes) {
        string s = "NO ONE";
        if (process.contributedBy != 0) {
            s = "Processor" + to_string(process.contributedBy);
        }
        fw << process.pid << "\t\t" << process.arrivalTime << "\t\t" << process.burstTime
           << "\t\t" << process.waitingTime << "\t\t" << process.turnaroundTime 
           << "\t\t\t" << process.completionTime << "\t\t\tProcessor " 
           << process.completedBy << "\t\t" << s << "\n";
    }
    fw << "================================================================================\n";
}

int MLFQ_2PROCESSOR::calculateTotalCompletionTime(const vector<Process>& processes) {
    int maxCompletionTime = 0;
    double averageWaitingTime = 0;
    double averageTurnAroundTime = 0;
    
    for (const auto& process : processes) {
        if (process.completionTime > maxCompletionTime) {
            maxCompletionTime = process.completionTime;
        }
        averageWaitingTime += process.waitingTime;
        averageTurnAroundTime += process.turnaroundTime;
    }
    
    if (!processes.empty()) {
        averageWaitingTime /= processes.size();
        averageTurnAroundTime /= processes.size();
    }
    
    fw << "\n=== PERFORMANCE METRICS ===\n";
    fw << "Average Waiting Time: " << averageWaitingTime << "\n";
    fw << "Average TurnAround Time: " << averageTurnAroundTime << "\n";
    fw << "Job Done by High Priority Queue: " << highQueueJobWorkDone << "\n";
    fw << "Job Done by Medium Priority Queue: " << mediumQueueJobWorkDone << "\n";
    fw << "Job Done by Low Priority Queue: " << lowQueueJobWorkDone << "\n";
    
    return maxCompletionTime;
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

void MLFQ_2PROCESSOR::execute() {
    cout << "=== Multi-Level Feedback Queue Scheduler (2 Processors) ===" << endl;
    cout << "Choose input method:" << endl;
    cout << "1) Terminal input" << endl;
    cout << "2) File input (input.txt)" << endl;
    cout << "Enter your choice (1 or 2): ";
    
    int choice;
    cin >> choice;
    
    vector<Process> processes;
    
    if (choice == 2) {
        processes = takeFileInput();
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
    
    cout << "\nRunning MLFQ scheduler with " << processes.size() << " processes..." << endl;
    
    // Reset static variables
    highQueueJobWorkDone = 0;
    mediumQueueJobWorkDone = 0;
    lowQueueJobWorkDone = 0;
    processMap.clear();
    
    // Run the scheduler
    run(processes, processes.size());
    
    // Write output to file
    fw.open("output.txt");
    if (!fw.is_open()) {
        cout << "Error: Could not create output.txt file!" << endl;
        return;
    }
    
    fw << "=== MULTI-LEVEL FEEDBACK QUEUE SCHEDULER RESULTS ===\n\n";
    printFinalStates(processes);
    
    int totalTime = calculateTotalCompletionTime(processes);
    fw << "\nTotal Time to Complete All Processes: " << totalTime << " units\n";
    
    fw.close();
    
    cout << "Execution completed successfully!" << endl;
    cout << "Results have been written to output.txt" << endl;
}

int main() {
    MLFQ_2PROCESSOR::execute();
    return 0;
}