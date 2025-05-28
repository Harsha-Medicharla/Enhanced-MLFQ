#ifndef MLFQ_2PROCESSOR_H
#define MLFQ_2PROCESSOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <string>

using namespace std;

class MLFQ_2PROCESSOR {
private:
    // Static variables for file writing and job tracking
    static ofstream fw;
    static int highQueueJobWorkDone;
    static int mediumQueueJobWorkDone;
    static int lowQueueJobWorkDone;

public:
    // Process structure to hold process information
    struct Process {
        int pid;                    // Process ID
        int arrivalTime;           // When process arrives
        int burstTime;             // Total execution time needed
        int remainingBurstTime;    // Remaining execution time
        int waitingTime;           // Time spent waiting
        int turnaroundTime;        // Total time from arrival to completion
        int completionTime;        // When process completes
        int completedBy;           // Which processor completed it (1 or 2)
        int contributedBy;         // Which processor helped (1 or 2)

        // Constructors
        Process();
        Process(int p, int at, int bt);
    };

    // Processor structure to represent each CPU
    struct Processor {
        int id;                    // Processor ID (1 or 2)
        bool isFree;              // Whether processor is free
        int totalLoad;            // Total load on processor
        queue<Process*> q1;       // High priority queue (time quantum = 5)
        queue<Process*> q2;       // Medium priority queue (time quantum = 8)
        queue<Process*> q3;       // Low priority queue (FCFS)

        // Constructor
        Processor(int i, bool free, int load);
    };

    // Map to track additional waiting time for processes
    static map<Process*, int> processMap;

    // Public methods
    static void takeInput(int n, vector<Process>& processes);
    static int remainingTime(Processor& processor);
    static void contribute(Processor& check, int id);
    static void run(vector<Process>& processes, int n);
    static void printFinalStates(const vector<Process>& processes);
    static int calculateTotalCompletionTime(const vector<Process>& processes);
    static vector<Process> takeTerminalInput();
    static vector<Process> takeFileInput();
    static void execute();
};

#endif // MLFQ_2PROCESSOR_H