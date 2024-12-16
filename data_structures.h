#ifndef DATA_STRUCTURES_H_
#define DATA_STRUCTURES_H_

#include "headers.h"
#include <stdio.h>

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

struct PriorityQueueNode {
    struct PCB proc;
    struct PriorityQueueNode* next;
};
struct PriorityQueue {
    struct PriorityQueueNode* head;
};
void initPriorityQueue(struct PriorityQueue* pq) {
    pq->head = NULL;
}
void push(struct PriorityQueue* pq, struct PCB new_proc, char criterion) {
    struct PriorityQueueNode* new_node = (struct PriorityQueueNode*)malloc(sizeof(struct PriorityQueueNode));
    if (!new_node) {
        printf("Error: Memory allocation failed\n");
        return;
    }
    new_node->proc = new_proc;
    new_node->next = NULL;

    // Determine the ordering criterion
    int compare;
    if (criterion == 'r') {
        compare = new_proc.remaining_time;
    } else {
        compare = new_proc.priority;
    }

    // Insert into the correct position in the queue
    if (pq->head == NULL || 
        (criterion == 'r' && new_proc.remaining_time < pq->head->proc.remaining_time) ||
        (criterion != 'r' && new_proc.priority < pq->head->proc.priority)) {
        // Insert at the head
        new_node->next = pq->head;
        pq->head = new_node;
    } else {
        // Traverse to find the correct position
        struct PriorityQueueNode* current = pq->head;
        while (current->next != NULL &&
               ((criterion == 'r' && current->next->proc.remaining_time <= new_proc.remaining_time) || 
                (criterion != 'r' && current->next->proc.priority <= new_proc.priority))) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}
struct PCB pop(struct PriorityQueue* pq) {
    if (pq->head == NULL) {
        printf("Error: Attempt to pop from an empty priority queue\n");
        struct PCB empty_proc;
        memset(&empty_proc, -2, sizeof(struct PCB));
        return empty_proc;
    }

    struct PriorityQueueNode* temp = pq->head;
    struct PCB proc = pq->head->proc;
    pq->head = pq->head->next;

    free(temp);
    return proc;
}

int isEmpty(struct PriorityQueue* pq) {
    return pq->head == NULL;
}

#endif