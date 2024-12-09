#include "headers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void clearResources(int);


int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // 1. Read the input files.
    char* file_path = argv[1];   
    FILE* f_ptr = NULL;
    f_ptr = fopen(file_path, "r");
    if(f_ptr == NULL){
        printf("%s not found\n", file_path);
        return -1;
    }

    char dump[40];
    fscanf(f_ptr, " %[^\n]", dump);
    unsigned num_of_processes = 0;
    while(fscanf(f_ptr, " %[^\n]", dump) != NULL){
        num_of_processes++;
    }
    

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int scheduling_alg; 
    int qunatum = -1;
    int shceduling_alg_index, quantum_index;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-sch")){
            if(i+1 == argc){
                printf("Scheduling algorithm number not provided\n");
                return -1;
            }else if(!strcmp(argv[i+1], "1") && !strcmp(argv[i+1], "2") && !strcmp(argv[i+1], "3") && !strcmp(argv[i+1], "4")){
                printf("Invalid scheduling algorithm number\n");
                return -1;
            }else{
                scheduling_alg = atoi(argv[i+1]);
                shceduling_alg_index = i+1;
            }
        }else if(strcmp(argv[i], "-q")){
            if(i+1 == argc){
                printf("No quantum number provided\n");
                return -1;
            }else{
                qunatum = atoi(argv[i+1]);
                quantum_index = i+1;
            }
        }
    }

    if((scheduling_alg != RR) && qunatum != -1){
        printf("Quantum parameter specified for an algorithm that doesn't need it\n");
        return -1;
    }else if((scheduling_alg == RR) && qunatum <= 0){
        printf("Invalid quantum number\n");
        return -1;
    }

    // 3. Initiate and create the scheduler and clock processes.
    char* clk_args = {"./clk.o", NULL};
    execv("clk.o", clk_args);

    int mqid = msgget(MSG_Q_KEY, 0644 | IPC_CREAT);
    if(mqid == -1){
        printf("Process generator failed to create message queue");
        return -1;
    }


    if(scheduling_alg == RR){ 
        char* scheduler_args[] = {"./scheduler.o", argv[1], "-sch", argv[shceduling_alg_index], "-q", argv[quantum_index], NULL};
        execv("scheduler.o", scheduler_args);
    }else{
        char* scheduler_args[] = {"./scheduler.o", argv[1], "-sch", argv[shceduling_alg_index], NULL};
        execv("scheduler.o", scheduler_args);
    }

    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);
    
    // 5. Create a data structure for processes and provide it with its parameters.
    struct PCB* processes = malloc(sizeof(struct PCB)*num_of_processes);
    unsigned id, arr_time, runtime, priority;
    rewind(f_ptr);
    fscanf(f_ptr, " %[^\n]", dump);
    int index = 0;
    while(fscanf(f_ptr, "%u   %u  %u  %u", &id, &arr_time, &runtime, &priority) != NULL){
        processes[index].pid = id;
        processes[index].arrival_time = arr_time;
        processes[index].remaining_time = runtime;
        processes[index].priority = priority;
        index++;
    }
    fclose(f_ptr);

    // 6. Send the information to the scheduler at the appropriate time.
    struct msgbuf sent_msg;
    int send_val;
    int processes_left = num_of_processes;
    int curr_time;
    while(processes_left > 0){
        curr_time = getClk();
        int i = 0;
        if(processes[i].arrival_time == curr_time){
            sent_msg.mtype = 0;
            sent_msg.proc.pid = processes[i].pid;
            sent_msg.proc.arrival_time = processes[i].arrival_time;
            sent_msg.proc.priority = processes[i].priority;
            sent_msg.proc.remaining_time = processes[i].remaining_time;
            sent_msg.proc.waiting_time = 0;
            send_val = msgsnd(mqid, &sent_msg, sizeof(struct PCB), !IPC_NOWAIT);
            if(send_val == -1){
                printf("Couldn't send process %u\n", processes[i].pid);
            }
            processes_left--;
            i++;
        }
    }
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}