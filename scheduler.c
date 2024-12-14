#include "headers.h"
#include "data_structures.h"
#include "helpers.h"
#include <sys/types.h>
#include <sys/msg.h>


struct MLFQ mlfq_ready_list;
int curr_priority_level = 0;

int queue_size = 0;
// not used by MLFQ
int last_element_index = -1;

void SJFSchedule(struct ProcLinkedListNode* queue);
void HPFSchedule(struct ProcLinkedListNode* queue);
void RRSchedule(struct ProcLinkedListNode* queue);
void MLFQSchedule(struct MLFQ* multilevel_queue);
void initializeMLFQ(struct MLFQ* multilevel_queue);
void SJFAddToReadyList(struct ProcLinkedListNode* queue, struct PCB proc);
void HPFAddToReadyList(struct ProcLinkedListNode* queue, struct PCB proc);
void RRAddToReadyList(struct ProcLinkedListNode* queue, struct PCB proc);
void MLFQAddToReadyList(struct MLFQ* multilevel_queue, struct PCB proc);

int main(int argc, char *argv[])
{
    int total_waiting_time = 0;
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

    initClk();

    //TODO: implement the scheduler.

    //TODO: upon termination release the clock resources.

    destroyClk(true);
}

void initializeMLFQ(struct MLFQ* multilevel_queue){
    for(int i = 0; i < NUM_OF_PRIORITY_LVLS; i++){
        multilevel_queue->queue_sizes[i] = 0;
        multilevel_queue->queues[i] = NULL;
    }
}

void MLFQAddToReadyList(struct MLFQ* multilevel_queue, struct PCB proc){
    pid_t pid;
    char remaining_time_str[5];
    for(int i = 0; i < NUM_OF_PRIORITY_LVLS; i++){
        if(proc.priority == i){ 
            struct MLFQLinkedListNode* temp;
            struct MLFQLinkedListNode* new_node;


            if(multilevel_queue->queues[proc.priority] == NULL){ // the queue was still empty
                multilevel_queue->queues[proc.priority] = new_node;
            }else{
                temp = multilevel_queue->queues[proc.priority];
                multilevel_queue->queues[proc.priority]->next_node = new_node;
                temp->next_node = new_node;
            }

            new_node = malloc(sizeof(struct MLFQLinkedListNode));
            new_node->next_node = NULL;
            new_node->current_priority = proc.priority;
            new_node->proc = proc;
            pid = fork();
            new_node->real_pid = pid;
            if(pid == -1){
                printf("Scheduler : failed to add process %d to the ready queue.\n", proc.pid);
                if(curr_node != NULL){ // the list wasn't already empty
                    free(new_node);
                    new_node = NULL;
                    curr_node->next_node = NULL;
                }else{
                    free(new_node);
                }
                multilevel_queue->queue_sizes[proc.priority] = multilevel_queue->queue_sizes[proc.priority] - 1;
            }else if(pid == 0){
                convertIntToStr(proc.remaining_time, 5, remaining_time_str);
                char* process_args[] = {"./process.o", remaining_time_str, NULL};
                execv("process.o", process_args);
            }

            if(pid != -1 && pid != 0){
                // do forget to do logging and calculations here 
            }

            multilevel_queue->queue_sizes[proc.priority] = multilevel_queue->queue_sizes[proc.priority] + 1;
            break;
        }
    }
}

void MLFQSchedule(struct MLFQ* multilevel_queue){

}