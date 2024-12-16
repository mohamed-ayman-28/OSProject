#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdlib.h>
#include"data_structures.h"
#include "headers.h"

int convertIntToStr(int i, int str_len, char* str){
    if(str_len <= 0 || str == NULL){
        return -1;
    }

    int index = str_len - 1;
    int digit;
    while(i > 0 && index > 0){
        digit = i % 10;
        str[index] = digit - '0';
        index--;
        i = i / 10;
    }
    str[str_len-1] = '\0';

    return 0; 
}


void schedulerLog(FILE* f_ptr, int curr_time, struct PCB* proc, int turnaround, double wta){
    switch (proc->state){
        case STARTING:
            fprintf(f_ptr, "#At time %d process %d %s arr %d total %d remain %d wait %d\n", curr_time, proc->pid, "started", proc->arrival_time, proc->running_time, proc->remaining_time, proc->waiting_time);
            break;
        case RESUMED:
            fprintf(f_ptr, "#At time %d process %d %s arr %d total %d remain %d wait %d\n", curr_time, proc->pid, "resumed", proc->arrival_time, proc->running_time, proc->remaining_time, proc->waiting_time);
            break;
        case STOPPED:
            fprintf(f_ptr, "#At time %d process %d %s arr %d total %d remain %d wait %d\n", curr_time, proc->pid, "stopped", proc->arrival_time, proc->running_time, proc->remaining_time, proc->waiting_time);
            break;
        case FINISHED:
            fprintf(f_ptr, "#At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f\n", curr_time, proc->pid, "resumed", proc->arrival_time, proc->running_time, proc->remaining_time, proc->waiting_time, turnaround, wta);
            break;
        default:
            break;
    }
}

#endif