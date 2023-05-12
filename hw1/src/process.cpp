#include "process.h"

Process process_table[MAX_PROCESSES];

Process* create_process(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == -1) { // Look for an empty entry in the process table
            process_table[i].pid = pid;
            process_table[i].state = READY;
            process_table[i].pc = 0;
            process_table[i].sp = 0;
            process_table[i].memory_alloc = 0;
            for (int j = 0; j < 10; j++) {
                process_table[i].open_files[j] = 0;
            }
            return &process_table[i];
        }
    }
    return nullptr; // Return nullptr if no empty entry is found
}

Process* get_process_by_pid(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            return &process_table[i];
        }
    }
    return nullptr; // Return nullptr if process is not found
}

void update_process_state(int pid, int state) {
    Process* process = get_process_by_pid(pid);
    if (process != nullptr) {
        process->state = state;
    }
}