#ifndef MLFQ_2PROCESSOR_H
#define MLFQ_2PROCESSOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <string>
#include <exception>
#include <iomanip>
#include <ctime>
#include <set>
#include <chrono>

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
        int responseTime;          // First response time
        bool hasStarted;           // Whether process has started execution

        // Constructors
        Process();
        Process(int p, int at, int bt);
    };

    // Processor structure to represent each CPU
    struct Processor {
        int id;                    // Processor ID (1 or 2)
        bool isFree;              // Whether processor is free
        int totalLoad;            // Total load on processor
        int currentTime;          // Current time for this processor
        queue<Process*> q1;       // High priority queue (time quantum = 5)
        queue<Process*> q2;       // Medium priority queue (time quantum = 8)
        queue<Process*> q3;       // Low priority queue (FCFS)

        // Constructor
        Processor(int i = 0, bool free = true, int load = 0, int time = 0);
    };



    // Map to track contribution time for processes
    static map<Process*, int> processContributionMap;

    // Public methods
    static vector<Process> takeTerminalInput();
    static vector<Process> takeFileInput();
    static void execute();
    static void resetStaticVariables();

    
private:
    // Private helper methods
    static void takeInput(int n, vector<Process>& processes);
    static int getTotalRemainingTime(Processor& processor);
    static void contributeToOtherProcessor(Processor& helper, Processor& target, int helperId);
    static void executeProcess(Process* process, Processor& processor, int timeQuantum, int& currentTime);
    static void run(vector<Process>& processes, int n);
    static void printFinalStates(const vector<Process>& processes);
    static void calculateAndPrintMetrics(const vector<Process>& processes);
    static string getCurrentDateTime();
};

#endif // MLFQ_2PROCESSOR_H