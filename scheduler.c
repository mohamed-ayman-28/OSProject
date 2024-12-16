#include "helpers.h"
#include <sys/msg.h>


struct MLFQ mlfq_ready_list;
struct PriorityQueue Readylist;
int curr_priority_level = 0;
int mqid;
struct PCB RunningProcess;
int queue_size = 0;
// not used by MLFQ
int last_element_index;
int quantum;
int curr_time;
int total_waiting_time, total_weighted_turnaround_time;
int num_of_processes;
double avg_waiting_time, avg_weighted_turnaround_time, cpu_utilization;
int start_time, end_time, idle_cycles;
FILE* scheduler_log_f_ptr;
FILE* scheduler_perf_f_ptr;

void SJFSchedule(struct PriorityQueue* queue);
void HPFSchedule(struct PriorityQueue* queue);
void RRSchedule(struct ProcLinkedListNode* queue);
void MLFQSchedule(struct MLFQ* multilevel_queue);
void initializeMLFQ(struct MLFQ* multilevel_queue);
void SJFAddToReadyList(struct PriorityQueue* queue, struct PCB proc);
void HPFAddToReadyList(struct PriorityQueue* queue, struct PCB proc);
void RRAddToReadyList(struct ProcLinkedListNode* queue, struct PCB proc);
void MLFQAddToReadyList(struct MLFQ* multilevel_queue, struct PCB proc);

int main(int argc, char *argv[])
{
    printf("d5lna l sched lolololoy \n ");

    mqid = msgget(MSG_Q_KEY, 0644);
    if(mqid == -1){
        printf("Scheduler failed to attach message queue");
        return -1;
    }

    total_waiting_time = 0;
    total_weighted_turnaround_time = 0;
    num_of_processes = 0;
    scheduler_log_f_ptr = NULL;
    idle_cycles = 0;
    int scheduling_alg; 
    quantum = -1;
    int shceduling_alg_index, quantum_index;

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-sch") == 0){
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
        }else if(strcmp(argv[i], "-q") == 0){
            if(i+1 == argc){
                printf("No quantum number provided\n");
                return -1;
            }else{
                quantum = atoi(argv[i+1]);
                quantum_index = i+1;
            }
        }
    }
    printf("shceduling_alg_index:%d\nshceduling_alg%d\n",shceduling_alg_index,scheduling_alg);
    if((scheduling_alg != RR && scheduling_alg != MLFQ) && quantum != -1){
        printf("Quantum parameter specified for an algorithm that doesn't need it\n");
        return -1;
    }else if((scheduling_alg == RR || scheduling_alg == MLFQ) && quantum <= 0){
        printf("Invalid quantum number\n");
        return -1;
    }

    initClk();

    //TODO: implement the scheduler.
    scheduler_log_f_ptr = fopen("scheduler.log", "w");
    fprintf(scheduler_log_f_ptr, "#At time x process y state arr w total z remain y wait k\n");
    printf("Ana hena dakhel el scheduler");
    //printf("%d\n",scheduling_alg);
    switch(scheduling_alg){
        case RR:
            break;
        case SJF:
            SJFSchedule(&Readylist);
            break;
        case HPF:
            HPFSchedule(&Readylist);
            break;
        case MLFQ:
            MLFQSchedule(&mlfq_ready_list);
            break;
        default:
            break;
    }

    avg_waiting_time = (double)total_waiting_time / (double)num_of_processes;
    avg_weighted_turnaround_time = (double)total_weighted_turnaround_time / (double)num_of_processes;
    cpu_utilization = (1 - (idle_cycles / (double)(end_time - start_time + 1))) * 100;

    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
void SJFAddToReadyList(struct PriorityQueue* queue, struct PCB proc)
{
     pid_t pid;
    char remaining_time_str[5];
    pid = fork();
    proc.real_pid = pid;
    if(pid == -1){
        printf("Scheduler : failed to add process %d to the ready queue.\n", proc.pid);
    }else if(pid == 0){
        convertIntToStr(proc.remaining_time, 5, remaining_time_str);
        char* const process_args[] = {(char* const)"./process.o", (char* const)remaining_time_str, NULL};
        execv("process.o", process_args);
        
    }
    else{
        num_of_processes++;
        push(queue, proc, 'r');
        printf("ana 5lst push l ID: %d\n ",proc.pid);
    }
}
void SJFSchedule(struct PriorityQueue* queue)
{
    //printf("Ana hena dakhalat el scheduler");
    initPriorityQueue(&Readylist);
    RunningProcess.remaining_time=-2;
    RunningProcess.pid=-2;
    int receivingFlag=0;
    struct msgbuf received_msg;
    int recv_val = -1;
    int turnaround_time;
    double weighted_turnaround_time;
    int waiting_time;
    start_time = getClk();
    //printf("SJF sched started");
    while (1) {

    // Wait for the next clock tick
    while (curr_time == getClk());
    curr_time = getClk();

    // Handle the running process
    if (RunningProcess.remaining_time >= 0) {
        printf("Running proc ID: %d\n Running proc Remaining time: %d\n", RunningProcess.pid, RunningProcess.remaining_time);
        if (RunningProcess.remaining_time == 0) {
            RunningProcess.state = FINISHED;
            turnaround_time = curr_time - RunningProcess.arrival_time;
            if (RunningProcess.running_time != 0) {
                weighted_turnaround_time = (float)turnaround_time / RunningProcess.running_time;
            } else {
                weighted_turnaround_time = 0;
            }
            waiting_time = turnaround_time - RunningProcess.running_time;
            total_waiting_time += waiting_time;
            total_weighted_turnaround_time += weighted_turnaround_time;
            schedulerLog(scheduler_log_f_ptr, curr_time, &(RunningProcess), turnaround_time, weighted_turnaround_time);

            // Pop the next process from the ready list
            RunningProcess = pop(&Readylist);
        } else {
            // Decrement remaining time and resume the process
            RunningProcess.remaining_time--;
            kill(RunningProcess.real_pid, SIGUSR1);
            RunningProcess.state = RESUMED;
            schedulerLog(scheduler_log_f_ptr, curr_time, &(RunningProcess), 0, 0);
        }
    } else if (!isEmpty(&Readylist)) {
        // Pick the next process if ready list is not empty
        RunningProcess = pop(&Readylist);
    }
    else{
        idle_cycles++;
    }

    // Reset recv_val and handle incoming messages
    recv_val = -1;
    while (recv_val == -1 && curr_time == getClk()) {
        recv_val = msgrcv(mqid, &received_msg, sizeof(struct PCB), 0, IPC_NOWAIT);
        if (received_msg.mtype==2)
        {
            receivingFlag=1;
        }
        
    }
    if (receivingFlag==1)
    {
        printf("1\n");
    }
    
    if (recv_val != -1 || receivingFlag==1) {
        if (received_msg.mtype == 1) {
            printf("Received message PID: %d\n", received_msg.proc.pid);
            SJFAddToReadyList(&Readylist, received_msg.proc);
        } else if (receivingFlag==1 && isEmpty(&Readylist)&& RunningProcess.remaining_time <= 0) {
            // Exit only if there are no remaining processes
            if (!isEmpty(&Readylist) || RunningProcess.remaining_time > 0) {
                continue; // Do not exit if there are still processes
            }
            printf("Exiting scheduler...\n");
            break;
        }
    }
}
end_time=curr_time;
received_msg.mtype=9;
msgsnd(mqid, &received_msg, sizeof(struct PCB), !IPC_NOWAIT);
}
void HPFAddToReadyList(struct PriorityQueue* queue, struct PCB proc)
{
     pid_t pid;
    char remaining_time_str[5];
    pid = fork();
    proc.real_pid = pid;
    if(pid == -1){
        printf("Scheduler : failed to add process %d to the ready queue.\n", proc.pid);
    }else if(pid == 0){
        convertIntToStr(proc.remaining_time, 5, remaining_time_str);
        char* const process_args[] = {(char* const)"./process.o", (char* const)remaining_time_str, NULL};
        execv("process.o", process_args);
        
    }
    else{
        num_of_processes++;
        push(queue, proc, 'p');
    }
}
void HPFSchedule(struct PriorityQueue* queue)
{
    //printf("Ana hena dakhalat el scheduler");
    initPriorityQueue(&Readylist);
    RunningProcess.remaining_time=-2;
    RunningProcess.pid=-2;
    int receivingFlag=0;
    struct msgbuf received_msg;
    int recv_val = -1;
    int turnaround_time;
    double weighted_turnaround_time;
    int waiting_time;
    start_time = getClk();
    //printf("SJF sched started");
    while (1) {

    // Wait for the next clock tick
    while (curr_time == getClk());
    curr_time = getClk();

    // Handle the running process
    if (RunningProcess.remaining_time >= 0) {
        printf("Running proc ID: %d\n Running proc Remaining time: %d\n", RunningProcess.pid, RunningProcess.remaining_time);
        if (RunningProcess.remaining_time == 0) {
            RunningProcess.state = FINISHED;
            turnaround_time = curr_time - RunningProcess.arrival_time;
            if (RunningProcess.running_time != 0) {
                weighted_turnaround_time = (float)turnaround_time / RunningProcess.running_time;
            } else {
                weighted_turnaround_time = 0;
            }
            waiting_time = turnaround_time - RunningProcess.running_time;
            total_waiting_time += waiting_time;
            total_weighted_turnaround_time += weighted_turnaround_time;
            schedulerLog(scheduler_log_f_ptr, curr_time, &(RunningProcess), turnaround_time, weighted_turnaround_time);

            // Pop the next process from the ready list
            RunningProcess = pop(&Readylist);
        } else {
            // Decrement remaining time and resume the process
            if(!isEmpty(&Readylist))
            {
                if(Readylist.head->proc.priority < RunningProcess.priority)
                {
                    printf("Ay haga msh hatefer2");
                    push(&Readylist,RunningProcess,'p');
                    RunningProcess = pop(&Readylist);
                }
            }
            RunningProcess.remaining_time--;
            kill(RunningProcess.real_pid, SIGUSR1);
            RunningProcess.state = RESUMED;
            schedulerLog(scheduler_log_f_ptr, curr_time, &(RunningProcess), 0, 0);
        }
    } else if (!isEmpty(&Readylist)) {
        // Pick the next process if ready list is not empty
        RunningProcess = pop(&Readylist);
    }
    else{
        idle_cycles++;
    }

    // Reset recv_val and handle incoming messages
    recv_val = -1;
    while (recv_val == -1 && curr_time == getClk()) {
        recv_val = msgrcv(mqid, &received_msg, sizeof(struct PCB), 0, IPC_NOWAIT);
        if (received_msg.mtype==2)
        {
            receivingFlag=1;
        }
        
    }
    if (receivingFlag==1)
    {
        printf("1\n");
    }
    
    if (recv_val != -1 || receivingFlag==1) {
        if (received_msg.mtype == 1) {
            printf("Received message PID: %d\n", received_msg.proc.pid);
            SJFAddToReadyList(&Readylist, received_msg.proc);
        } else if (receivingFlag==1 && isEmpty(&Readylist)&& RunningProcess.remaining_time <= 0) {
            // Exit only if there are no remaining processes
            if (!isEmpty(&Readylist) || RunningProcess.remaining_time > 0) {
                continue; // Do not exit if there are still processes
            }
            printf("Exiting scheduler...\n");
            break;
        }
    }
}
end_time=curr_time;
received_msg.mtype=9;
msgsnd(mqid, &received_msg, sizeof(struct PCB), !IPC_NOWAIT);
}

void initializeMLFQ(struct MLFQ* multilevel_queue){
    for(int i = 0; i < NUM_OF_PRIORITY_LVLS; i++){
        multilevel_queue->queues[i] = NULL;
        multilevel_queue->queue_curr_node[i] = NULL;
        multilevel_queue->queue_last_node[i] = NULL;
    }
}

void MLFQAddToReadyList(struct MLFQ* multilevel_queue, struct PCB proc){
    pid_t pid;
    char remaining_time_str[5];
    struct MLFQLinkedListNode* temp;
    struct MLFQLinkedListNode* new_node;

    new_node = malloc(sizeof(struct MLFQLinkedListNode));
    new_node->next_node = NULL;
    new_node->current_priority = proc.priority;
    new_node->proc = proc;
    new_node->cycles_left_in_quantum = quantum;

    pid = fork();
    new_node->real_pid = pid;
    if(pid == -1){
        printf("Scheduler : failed to add process %d to the ready queue.\n", proc.pid);
        free(new_node);
    }else if(pid == 0){
        convertIntToStr(proc.remaining_time, 5, remaining_time_str);
        char* const process_args[] = {(char* const)"./process.o", (char* const)remaining_time_str, NULL};
        execv("process.o", process_args);
        num_of_processes++;
    }
    if(pid != -1 && pid != 0){
        if(multilevel_queue->queue_last_node[proc.priority] == NULL){ // list is empty
            multilevel_queue->queue_last_node[proc.priority] = new_node; // set the last node of the queue
            multilevel_queue->queues[proc.priority] = new_node;          // set the first node of the queue
        }else{
            multilevel_queue->queue_last_node[proc.priority]->next_node = new_node;// set the last node of the queue
            multilevel_queue->queue_last_node[proc.priority] = new_node;
        }
        schedulerLog(scheduler_log_f_ptr, curr_time, &(new_node->proc), 0, 0);
    }

}


void MLFQSchedule(struct MLFQ* multilevel_queue){


    struct msgbuf received_msg;
    int recv_val = -1;
    curr_time = -1;
    int no_scheduling;
    int no_process_added;

    struct MLFQLinkedListNode* temp;
    struct MLFQLinkedListNode* temp2;
    int queue_is_empty;
    int turnaround_time;
    double weighted_turnaround_time;
    int waiting_time;
    start_time = getClk();
    while(1){
        no_scheduling = 1;
        no_process_added = 1;
        while(curr_time == getClk());
        curr_time = getClk();

        // reach to the first non-empty queue
        while(multilevel_queue->queues[curr_priority_level] == NULL && curr_priority_level <= NUM_OF_PRIORITY_LVLS){
            curr_priority_level++;
        }


        if(curr_priority_level <= NUM_OF_PRIORITY_LVLS){ // if this branch is taken, then that level must not be empty
            // curr_node=NULL either because it has not been set yet to point to any node or because
            // it now points to the end of the quque.  In both cases, it will point to the  
            // first node in the queue (the earliest node in the queue)
            no_scheduling = 0;
            if(multilevel_queue->queue_curr_node[curr_priority_level] == NULL){
                multilevel_queue->queue_curr_node[curr_priority_level] = multilevel_queue->queues[curr_priority_level]; 
            }
 
            if(multilevel_queue->queue_curr_node[curr_priority_level]->proc.remaining_time <= 0){ // the process finsished
                queue_is_empty = (multilevel_queue->queue_last_node[curr_priority_level] == multilevel_queue->queues[curr_priority_level])?1:0;
                temp = multilevel_queue->queues[curr_priority_level]; 
                while(temp->next_node != multilevel_queue->queue_curr_node[curr_priority_level]){
                    temp = temp->next_node;
                }
                // now temp point to the node before the current node
                // remove the current node from this level
                temp2 = multilevel_queue->queue_curr_node[curr_priority_level];
                temp->next_node = multilevel_queue->queue_curr_node[curr_priority_level]->next_node;
                multilevel_queue->queue_curr_node[curr_priority_level]->proc.state = FINISHED;
                // log to scheduler.perf
                turnaround_time = curr_time - temp2->proc.arrival_time;
                weighted_turnaround_time = turnaround_time / temp2->proc.running_time;
                waiting_time = turnaround_time - temp2->proc.running_time;
                total_waiting_time = total_waiting_time + waiting_time;
                total_weighted_turnaround_time = total_weighted_turnaround_time + weighted_turnaround_time;
                schedulerLog(scheduler_log_f_ptr, curr_time, &(multilevel_queue->queue_curr_node[curr_priority_level]->proc), turnaround_time, weighted_turnaround_time);

                multilevel_queue->queue_curr_node[curr_priority_level] = multilevel_queue->queue_curr_node[curr_priority_level]->next_node;
                

                free(temp2);
                if(queue_is_empty){
                    multilevel_queue->queue_curr_node[curr_priority_level] = NULL;
                    multilevel_queue->queue_last_node[curr_priority_level] = NULL;
                    multilevel_queue->queues[curr_priority_level] = NULL;
                }

                curr_priority_level = (curr_priority_level + 1)%(NUM_OF_PRIORITY_LVLS + 1);
            }else if (multilevel_queue->queue_curr_node[curr_priority_level]->cycles_left_in_quantum <= 0){
                // set the curr_node to point to the next node
                queue_is_empty = (multilevel_queue->queue_last_node[curr_priority_level] == multilevel_queue->queues[curr_priority_level])?1:0;
                temp = multilevel_queue->queues[curr_priority_level]; 
                while(temp->next_node != multilevel_queue->queue_curr_node[curr_priority_level]){
                    temp = temp->next_node;
                }
                temp2 = multilevel_queue->queue_curr_node[curr_priority_level];
                multilevel_queue->queue_curr_node[curr_priority_level]->proc.state = STOPPED;
                schedulerLog(scheduler_log_f_ptr, curr_time, &(multilevel_queue->queue_curr_node[curr_priority_level]->proc), 0, 0);
                // now temp point to the node before the current node, and temp2 points to the current node
                
                // remove the current node from this level
                temp->next_node = multilevel_queue->queue_curr_node[curr_priority_level]->next_node; 
                multilevel_queue->queue_curr_node[curr_priority_level] = multilevel_queue->queue_curr_node[curr_priority_level]->next_node;
                temp2->next_node = NULL;
                if(queue_is_empty){
                    multilevel_queue->queue_curr_node[curr_priority_level] = NULL;
                    multilevel_queue->queue_last_node[curr_priority_level] = NULL;
                    multilevel_queue->queues[curr_priority_level] = NULL;
                }
    
                // insert it in the next level
                curr_priority_level = (curr_priority_level + 1)%(NUM_OF_PRIORITY_LVLS + 1);
                temp2->cycles_left_in_quantum = quantum;
                if(multilevel_queue->queues[curr_priority_level] != NULL){
                    multilevel_queue->queues[curr_priority_level] = temp2;
                }else{
                    multilevel_queue->queue_last_node[curr_priority_level]->next_node = temp2;
                } 
            }else{
                // give that process a quantum
                if(multilevel_queue->queue_curr_node[curr_priority_level]->proc.last_time_running < curr_time-1){
                    multilevel_queue->queue_curr_node[curr_priority_level]->proc.waiting_time = multilevel_queue->queue_curr_node[curr_priority_level]->proc.waiting_time + (curr_time - multilevel_queue->queue_curr_node[curr_priority_level]->proc.last_time_running); 
                }
                multilevel_queue->queue_curr_node[curr_priority_level]->proc.remaining_time = multilevel_queue->queue_curr_node[curr_priority_level]->proc.remaining_time - 1;
                multilevel_queue->queue_curr_node[curr_priority_level]->cycles_left_in_quantum = multilevel_queue->queue_curr_node[curr_priority_level]->cycles_left_in_quantum - 1; 
                kill(multilevel_queue->queue_curr_node[curr_priority_level]->real_pid, SIGUSR1);
                multilevel_queue->queue_curr_node[curr_priority_level]->proc.state = RESUMED;
                schedulerLog(scheduler_log_f_ptr, curr_time, &(multilevel_queue->queue_curr_node[curr_priority_level]->proc), 0, 0);
                multilevel_queue->queue_curr_node[curr_priority_level]->proc.last_time_running = curr_time;
            }
        }else{
            curr_priority_level = 0;
        }

        // attempt to read from the queue for the whole cycle
        while(recv_val == -1 && curr_time == getClk()){
            recv_val = msgrcv(mqid, &received_msg, sizeof(struct PCB), 0, IPC_NOWAIT);
        }

        if(recv_val != -1 && received_msg.mtype == 1){ // if exiting the while loop is because a process has been read from the queue at this cycle, add this process to the read queue 
            MLFQAddToReadyList(multilevel_queue, received_msg.proc);
            no_process_added = 0;
        }else if(recv_val != -1 && received_msg.mtype == 2){
            break;
        }
        
        if(no_process_added && no_scheduling){ // the processor was idle in this cycle
            idle_cycles++;
        }
    }
 received_msg.mtype=9;
msgsnd(mqid, &received_msg, sizeof(struct PCB), !IPC_NOWAIT);
end_time = curr_time;
    
}