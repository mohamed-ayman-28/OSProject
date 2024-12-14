#include "headers.h"

struct ProcLinkedListNode {
    int real_pid;
    struct PCB proc;
    struct ProcLinkedListNode* next_node;
};

struct MLFQLinkedListNode {
    int real_pid;
    struct PCB proc;
    int current_priority;
    struct MLFQLinkedListNode* next_node;
};

struct MLFQ{
    int queue_sizes[NUM_OF_PRIORITY_LVLS];
    struct MLFQLinkedListNode* queues[NUM_OF_PRIORITY_LVLS];
    struct MLFQLinkedListNode* curr_node_in_queue[NUM_OF_PRIORITY_LVLS]; // each element points to the current element in that queue
};
