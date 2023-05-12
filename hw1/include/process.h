#ifndef PROCESS_H
#define PROCESS_H
// Define process state constants

enum ProcessState { 
    READY,
    RUNNING,
    BLOCKED,
    MAX_PROCESSES = 10
};
// Define process structure

struct Process {
    int pid; // Process ID
    int state;  // Process state (READY, RUNNING, or BLOCKED)
    int pc;  // Program counter
    int sp;  // Stack pointer
    int memory_alloc; // Memory allocation
    int open_files[10]; // Status of open files
    int ppid;
    int priority;
    double cpu_time;
};
Process* create_process(int pid);
Process* get_process_by_pid(int pid);
void update_process_state(int pid, int state);

#endif