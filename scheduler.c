#include "headers.h"
#include <sys/types.h>
#include <sys/msg.h>

struct PCB* ready_queue;
int queue_size = 0;
int last_element_index = -1;

void SJFSchedule(struct PCB* queue);
void HPFSchedule(struct PCB* queue);
void RRSchedule(struct PCB* queue);
void MLFLSchedule(struct PCB* queue);
void SJFAddToReadyList(struct PCB* queue, struct PCB proc);
void HPFAddToReadyList(struct PCB* queue, struct PCB proc);
void RRAddToReadyList(struct PCB* queue, struct PCB proc);
void MLFLAddToReadyList(struct PCB* queue, struct PCB proc);

int main(int argc, char *argv[])
{
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


void SJFAddToReadyList(struct PCB* queue, struct PCB proc){
    int i;
    for(i = last_element_index; i >= 0 ; i--){
        if(queue[i].remaining_time <= proc.remaining_time){
            queue[i+1] = queue[i];
        }else{
            break;
        }
    }

    queue[i+1] = proc;    
}
