#ifndef EX6_DESIGN_PATTERNS_QUEUE_H
#define EX6_DESIGN_PATTERNS_QUEUE_H
#endif //EX6_DESIGN_PATTERNS_QUEUE_H
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*
 * This queue was taken from https://www.geeksforgeeks.org/queue-linked-list-implementation/
 */

// A linked list (LL) node to store a queue entry
struct QNode {
    char* key;
    struct QNode* next;
    int sock; //For sending back the data to the client
};
// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue {
    struct QNode *front, *rear;
    int id;
    int status;
    pthread_mutex_t mutexQ;
    pthread_cond_t condQ;

};

// A utility function to create a new linked list node.
struct QNode* newNode(char* k){
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue* createQ(){
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}
void enQ(struct Queue* q, struct QNode *k){
    pthread_mutex_lock(&(q->mutexQ));
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = k;
        pthread_cond_signal(&(q->condQ));
        pthread_mutex_unlock(&(q->mutexQ));
        return;
    }
    // Add the new node at the end of queue and change rear
    q->rear->next = k;
    q->rear = k;
    pthread_mutex_unlock(&(q->mutexQ));
}

struct QNode* deQ(struct Queue* q){
    // If queue is empty, waiting for enQ
    pthread_mutex_lock(&(q->mutexQ));
    while (q->front == NULL){
        printf("Queue %d is empty... waiting for enQ\n",q->id);
        pthread_cond_wait(&(q->condQ),&(q->mutexQ));
        if (q->status==-1){
            return NULL;
        }
    }
    printf("enQ to queue %d has been made\n",q->id);
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
    q->front = q->front->next;
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    pthread_mutex_unlock(&(q->mutexQ));
    return temp;
}

struct QNode* lastDeQ(struct Queue* q){
    // If queue is empty, waiting for enQ
    while (q->front == NULL){
        printf("Queue %d is empty...\n",q->id);
            return NULL;
    }
    printf("enQ to queue %d has been made\n",q->id);
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
    q->front = q->front->next;
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    return temp;
}

// free each Node in the queue and after free the queue.
void destroyQ(struct Queue* q){
    printf("destroyQ %d\n",q->id);
    q->status=-1;
    pthread_cond_signal(&(q->condQ));
    while (q->front!=NULL){
        struct QNode* temp = lastDeQ(q);
        free(temp);
    }
    pthread_mutex_destroy(&(q->mutexQ));
    pthread_cond_destroy(&(q->condQ));
    free(q);
}


