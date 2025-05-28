#include "MLFQ_2PROCESSOR.h"

// Constructor implementations
MLFQ_2PROCESSOR::Process::Process() : pid(0), arrivalTime(0), burstTime(0), remainingBurstTime(0), 
                                     waitingTime(0), turnaroundTime(0), completionTime(0), 
                                     completedBy(0), contributedBy(0) {}

MLFQ_2PROCESSOR::Process::Process(int p, int at, int bt) : pid(p), arrivalTime(at), burstTime(bt), 
                                                          remainingBurstTime(bt), waitingTime(0), 
                                                          turnaroundTime(0), completionTime(0), 
                                                          completedBy(0), contributedBy(0) {}

MLFQ_2PROCESSOR::Processor::Processor(int i, bool free, int load) : id(i), isFree(free), totalLoad(load) {}

    static void takeInput(int n, vector<Process>& processes) {
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

    static int remainingTime(Processor& processor) {
        int remainingTime = 0;
        
        queue<Process*> tempQ1 = processor.q1;
        while (!tempQ1.empty()) {
            remainingTime += tempQ1.front()->remainingBurstTime;
            tempQ1.pop();
        }
        
        queue<Process*> tempQ2 = processor.q2;
        while (!tempQ2.empty()) {
            remainingTime += tempQ2.front()->remainingBurstTime;
            tempQ2.pop();
        }
        
        queue<Process*> tempQ3 = processor.q3;
        while (!tempQ3.empty()) {
            remainingTime += tempQ3.front()->remainingBurstTime;
            tempQ3.pop();
        }
        
        return remainingTime;
    }

    static void contribute(Processor& check, int id) {
        if (!check.q1.empty()) {
            Process* p = check.q1.front();
            if (p->remainingBurstTime > 1) {
                p->remainingBurstTime--;
                highQueueJobWorkDone++;
                processMap[p] = processMap[p] + 1;
                p->contributedBy = id;
            }
        } else if (!check.q2.empty()) {
            Process* p = check.q2.front();
            if (p->remainingBurstTime > 1) {
                p->remainingBurstTime--;
                mediumQueueJobWorkDone++;
                processMap[p] = processMap[p] + 1;
                p->contributedBy = id;
            }
        } else if (!check.q3.empty()) {
            Process* p = check.q3.front();
            if (p->remainingBurstTime > 1) {
                lowQueueJobWorkDone++;
                p->remainingBurstTime--;
                processMap[p] = processMap[p] + 1;
                p->contributedBy = id;
            }
        }
    }

    static void run(vector<Process>& processes, int n) {
        Processor processor1(1, true, 0);
        Processor processor2(2, true, 0);

        int time1 = 0;
        int time2 = 0;

        if (n == 0) {
            cout << "Program doesn't run with 0 processes" << endl;
            return;
        }

        int completed = 0;
        int time = 0;
        queue<Process*> mainQueue;

        for (auto& process : processes) {
            if (process.arrivalTime == 0) {
                if (remainingTime(processor1) <= remainingTime(processor2)) {
                    processor1.q1.push(&process);
                } else {
                    processor2.q1.push(&process);
                }
            } else {
                mainQueue.push(&process);
            }
        }

        if (processor1.q1.empty() && processor2.q1.empty()) {
            if (!mainQueue.empty() && mainQueue.front()->arrivalTime > 0) {
                time1 = mainQueue.front()->arrivalTime;
                time2 = time1;
                time = time1;
            }
        }

        for (; time >= 0; time++) {
            if (completed >= n) break;
            bool check1 = true;
            bool check2 = true;

            // Processor 1 logic
            if (time == time1) {
                while (!mainQueue.empty() && mainQueue.front()->arrivalTime <= time1) {
                    Process* process = mainQueue.front();
                    mainQueue.pop();
                    if (remainingTime(processor1) <= remainingTime(processor2)) {
                        processor1.q1.push(process);
                    } else {
                        processor2.q1.push(process);
                    }
                }

                if (!processor1.q1.empty()) {
                    Process* p = processor1.q1.front();
                    if (p->arrivalTime > time1) {
                        time1++;
                        check1 = false;
                    }
                    if (check1) {
                        if (p->remainingBurstTime <= 5) {
                            p->completionTime = time1 + p->remainingBurstTime;
                            p->turnaroundTime = p->completionTime - p->arrivalTime;
                            p->waitingTime = p->turnaroundTime - p->burstTime;
                            time1 += p->remainingBurstTime;
                            if (processMap.find(p) != processMap.end()) {
                                p->waitingTime += processMap[p];
                            }
                            highQueueJobWorkDone += p->remainingBurstTime;
                            processor1.q1.pop();
                            completed++;
                            p->completedBy = 1;
                        } else {
                            p->remainingBurstTime -= 5;
                            highQueueJobWorkDone += 5;
                            time1 += 5;
                            processor1.q2.push(p);
                            processor1.q1.pop();
                        }
                    }
                } else if (!processor1.q2.empty() && check1) {
                    while (!mainQueue.empty() && mainQueue.front()->arrivalTime <= time1) {
                        if (remainingTime(processor1) <= remainingTime(processor2)) {
                            processor1.q1.push(mainQueue.front());
                            mainQueue.pop();
                            check1 = false;
                        } else {
                            processor2.q1.push(mainQueue.front());
                            mainQueue.pop();
                        }
                    }
                    if (check1) {
                        Process* p = processor1.q2.front();
                        if (p->remainingBurstTime <= 8) {
                            p->completionTime = time1 + p->remainingBurstTime;
                            p->turnaroundTime = p->completionTime - p->arrivalTime;
                            p->waitingTime = p->turnaroundTime - p->burstTime;
                            time1 += p->remainingBurstTime;
                            if (processMap.find(p) != processMap.end()) {
                                p->waitingTime += processMap[p];
                            }
                            mediumQueueJobWorkDone += p->remainingBurstTime;
                            processor1.q2.pop();
                            completed++;
                            p->completedBy = 1;
                        } else {
                            p->remainingBurstTime -= 8;
                            time1 += 8;
                            mediumQueueJobWorkDone += 8;
                            processor1.q3.push(p);
                            processor1.q2.pop();
                        }
                    }
                } else if (!processor1.q3.empty() && check1) {
                    Process* p = processor1.q3.front();
                    while (p->remainingBurstTime != 0) {
                        check1 = false;
                        while (!mainQueue.empty() && mainQueue.front()->arrivalTime <= time1) {
                            if (remainingTime(processor1) <= remainingTime(processor2)) {
                                processor1.q2.push(p);
                                processor1.q3.pop();
                                Process* process = mainQueue.front();
                                mainQueue.pop();
                                processor1.q1.push(process);
                                break;
                            } else {
                                processor2.q1.push(mainQueue.front());
                                mainQueue.pop();
                            }
                        }
                        if (time < time1) break;
                        p->completionTime = 1 + time1;
                        lowQueueJobWorkDone++;
                        p->remainingBurstTime--;
                        time1++;
                    }
                    if (check1) {
                        p->turnaroundTime = p->completionTime - p->arrivalTime;
                        p->waitingTime = p->turnaroundTime - p->burstTime;
                        processor1.q3.pop();
                        if (processMap.find(p) != processMap.end()) {
                            p->waitingTime += processMap[p];
                        }
                        completed++;
                        p->completedBy = 1;
                    }
                } else {
                    time1++;
                    contribute(processor2, 1);
                }
            }

            // Processor 2 logic
            if (time == time2) {
                while (!mainQueue.empty() && mainQueue.front()->arrivalTime <= time2) {
                    Process* process = mainQueue.front();
                    mainQueue.pop();
                    if (remainingTime(processor2) <= remainingTime(processor1)) {
                        processor2.q1.push(process);
                    } else {
                        processor1.q1.push(process);
                    }
                }

                if (!processor2.q1.empty()) {
                    Process* p = processor2.q1.front();
                    if (p->arrivalTime > time2) {
                        time2++;
                        check2 = false;
                    }
                    if (check2) {
                        if (p->remainingBurstTime <= 5) {
                            p->completionTime = time2 + p->remainingBurstTime;
                            p->turnaroundTime = p->completionTime - p->arrivalTime;
                            p->waitingTime = p->turnaroundTime - p->burstTime;
                            time2 += p->remainingBurstTime;
                            highQueueJobWorkDone += p->remainingBurstTime;
                            processor2.q1.pop();
                            if (processMap.find(p) != processMap.end()) {
                                p->waitingTime += processMap[p];
                            }
                            completed++;
                            p->completedBy = 2;
                        } else {
                            p->remainingBurstTime -= 5;
                            time2 += 5;
                            processor2.q2.push(p);
                            processor2.q1.pop();
                            highQueueJobWorkDone += 5;
                        }
                    }
                } else if (!processor2.q2.empty() && check2) {
                    while (!mainQueue.empty() && mainQueue.front()->arrivalTime <= time2) {
                        Process* process = mainQueue.front();
                        mainQueue.pop();
                        if (remainingTime(processor2) <= remainingTime(processor1)) {
                            processor2.q1.push(process);
                            check2 = false;
                        } else {
                            processor1.q1.push(process);
                        }
                    }
                    if (check2) {
                        Process* p = processor2.q2.front();
                        if (p->remainingBurstTime <= 8) {
                            p->completionTime = time2 + p->remainingBurstTime;
                            p->turnaroundTime = p->completionTime - p->arrivalTime;
                            p->waitingTime = p->turnaroundTime - p->burstTime;
                            time2 += p->remainingBurstTime;
                            mediumQueueJobWorkDone += p->remainingBurstTime;
                            processor2.q2.pop();
                            if (processMap.find(p) != processMap.end()) {
                                p->waitingTime += processMap[p];
                            }
                            completed++;
                            p->completedBy = 2;
                        } else {
                            p->remainingBurstTime -= 8;
                            time2 += 8;
                            processor2.q3.push(p);
                            processor2.q2.pop();
                            mediumQueueJobWorkDone += 8;
                        }
                    }
                } else if (!processor2.q3.empty() && check2) {
                    Process* p = processor2.q3.front();
                    while (p->remainingBurstTime != 0) {
                        check2 = false;
                        while (!mainQueue.empty() && mainQueue.front()->arrivalTime <= time2) {
                            if (remainingTime(processor2) <= remainingTime(processor1)) {
                                processor2.q2.push(p);
                                processor2.q3.pop();
                                Process* process = mainQueue.front();
                                mainQueue.pop();
                                processor2.q1.push(process);
                                break;
                            } else {
                                Process* process = mainQueue.front();
                                mainQueue.pop();
                                processor1.q1.push(process);
                            }
                        }
                        if (time < time2) break;
                        p->completionTime = 1 + time2;
                        lowQueueJobWorkDone++;
                        p->remainingBurstTime--;
                        time2++;
                    }
                    if (check2) {
                        p->turnaroundTime = p->completionTime - p->arrivalTime;
                        p->waitingTime = p->turnaroundTime - p->burstTime;
                        processor2.q3.pop();
                        if (processMap.find(p) != processMap.end()) {
                            p->waitingTime += processMap[p];
                        }
                        completed++;
                        p->completedBy = 2;
                    }
                } else {
                    time2++;
                    contribute(processor1, 2);
                }
            }
        }
    }

    static void printFinalStates(const vector<Process>& processes) {
        fw << "PID\t\tAT\t\tBT\t\tWT\t\tTurnT\t\tCompT\t\tDone By\t\tHelped By\n";
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
    }

    static int calculateTotalCompletionTime(const vector<Process>& processes) {
        int max = 0;
        double averageWaitingTime = 0;
        double turnAroundTime = 0;
        
        for (const auto& process : processes) {
            if (process.completionTime > max) {
                max = process.completionTime;
            }
            averageWaitingTime += process.waitingTime;
            turnAroundTime += process.turnaroundTime;
        }
        
        fw << "Average Waiting Time : " << averageWaitingTime / processes.size() << "\n";
        fw << "Average TurnAround Time : " << turnAroundTime / processes.size() << "\n";
        fw << "Job Done by HighLevel Queue: " << highQueueJobWorkDone << "\n";
        fw << "Job Done by MidLevel Queue: " << mediumQueueJobWorkDone << "\n";
        fw << "Job Done by Lowlevel Queue: " << lowQueueJobWorkDone << "\n";
        
        return max;
    }

    static vector<Process> takeTerminalInput() {
        cout << "Enter the number of processes: ";
        int n;
        cin >> n;
        vector<Process> processes(n);
        
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
        
        return processes;
    }

    static vector<Process> takeFileInput() {
        try {
            ifstream file("input.txt");
            if (!file.is_open()) {
                cout << "File not found" << endl;
                return vector<Process>();
            }
            
            int n;
            file >> n;
            vector<Process> processes(n);
            
            for (int i = 0; i < n; i++) {
                int arrivalTime, burstTime;
                file >> arrivalTime >> burstTime;
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

    static void execute() {
        int t;
        cout << "Choose the way you want to give input: \n1) terminal \n2) file \n";
        cin >> t;
        
        vector<Process> processes;
        if (t == 2) {
            processes = takeFileInput();
        } else {
            processes = takeTerminalInput();
            cout << "Check the output file for output..." << endl;
        }
        
        if (processes.empty()) {
            cout << "No processes to execute" << endl;
            return;
        }
        
        // Run the processes
        run(processes, processes.size());
        
        // Write output to file
        fw.open("output.txt");
        if (!fw.is_open()) {
            cout << "Error opening output file" << endl;
            return;
        }
        
        printFinalStates(processes);
        
        int max = calculateTotalCompletionTime(processes);
        fw << "\n Total Time to Complete all process is : " << max << "\n";
        fw.close();
        
        cout << "Output written to output.txt" << endl;
    }
};

// Static member definitions
ofstream MLFQ_2PROCESSOR::fw;
int MLFQ_2PROCESSOR::highQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::mediumQueueJobWorkDone = 0;
int MLFQ_2PROCESSOR::lowQueueJobWorkDone = 0;
map<MLFQ_2PROCESSOR::Process*, int> MLFQ_2PROCESSOR::processMap;

int main() {
    MLFQ_2PROCESSOR::execute();
    return 0;
}