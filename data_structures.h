#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#include "headers.h"

struct ProcLinkedListNode {
    int real_pid;
    struct PCB proc;
    struct ProcLinkedListNode* next_node;
};

struct MLFQLinkedListNode {
    int cycles_left_in_quantum;
    int real_pid;
    struct PCB proc;
    int current_priority;
    struct MLFQLinkedListNode* next_node;
};

struct MLFQ{
    struct MLFQLinkedListNode* queues[NUM_OF_PRIORITY_LVLS];
    struct MLFQLinkedListNode* queue_curr_node[NUM_OF_PRIORITY_LVLS]; // each element points to the current element in that queue
    struct MLFQLinkedListNode* queue_last_node[NUM_OF_PRIORITY_LVLS];
};

#endif