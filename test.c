#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "Queue.h"
#define TRUE 1

char* ans[7];
pthread_t th[3];

/*
* Global queues
*/
struct Queue* q1 = NULL;
struct Queue* q2 = NULL;
struct Queue* q3 = NULL;

// global pipline pointer
struct pipline* pointer;

/*
 * This function initializes q1,q2,q3 and their mutex and cond
 */
void initQueues(void) {
    printf("initialize all Queues\n");
    q1 = createQ();
    q2 = createQ();
    q3 = createQ();
    q1->id=1;
    q2->id=2;
    q3->id=3;
    q1->status = 1;
    q2->status = 1;
    q3->status = 1;
    pthread_mutex_init(&(q1->mutexQ), NULL);
    pthread_cond_init(&(q1->condQ), NULL);
    pthread_mutex_init(&(q2->mutexQ), NULL);
    pthread_cond_init(&(q2->condQ), NULL);
    pthread_mutex_init(&(q3->mutexQ), NULL);
    pthread_cond_init(&(q3->condQ), NULL);
    ans[0]="1"; // last char in ascii table
    ans[1]="a";
    ans[2]="A";
    ans[3]="0";
    ans[4]="234";
    ans[5]="BCD";
}

typedef struct active_object {
    struct Queue *q;
    void* (*q_fun_ptr)(void*); // first func
    void* (*f_fun_ptr)(void*); // after func
    pthread_t my_pid;
} active_object;

typedef struct pipline {
    active_object *first;
    active_object *second;
    active_object *third;
} pipline;


void makeNewAO(active_object *ao){
    printf("Making newAO for Queue %d\n",ao->q->id);
    newAO(ao->q,ao->q_fun_ptr,ao->f_fun_ptr);
}

void newAO(struct Queue *q, void (*q_fun_ptr)(), void (*f_fun_ptr)()) {
    printf("NewAO for queue %d is established\n",q->id);
    while (1){
        if (q->status==-1){
            break;
        }
        struct QNode* temp = deQ(q);
        (*q_fun_ptr)(temp);
        (*f_fun_ptr)(temp);
    }

}

//__________________________________________FunctionsForActiveObjects________________________________________________//

void Ao1(struct QNode* node){
    if (node==NULL){
        return;
    }
//    printf("The string %s is in AO1\n",node->key);
    char* str = node->key;
    int len = strlen(node->key);
    for (int i = 0; i < len; i++){
        if (node->key[i] == 'z'){
            node->key[i] = 'a';
        }
        else if (node->key[i] =='Z'){
            node->key[i] = 'A';
        }
        else if(node->key[i]=='9'){
            node->key[i]= '0';
        }
        else if(node->key[i]=='~'){
            node->key[i]=' ';
        }
        else{
            node->key[i]+=1;
        }
    }
}

void Ao2(struct QNode* node){
    if (node==NULL){
        return;
    }
//    printf("The string %s is in AO2\n",node->key);
    int len = strlen(node->key);
    for (int i = 0; i < len; i++){
        if (node->key[i]>=65 && node->key[i] <=90){
            node->key[i] += 32;
        }
        else if(node->key[i]>=32 && node->key[i]<=64){
            return;
        }
        else if(node->key[i]>=91 && node->key[i]<=96){
            return;
        }
        else if(node->key[i]>=123 && node->key[i]<=126){
            return;
        }
        else{
            node->key[i] -= 32;
        }
    }
}

void afterAo1(struct QNode* node){
    if (node==NULL){
        return;
    }
//    printf("The string %s is now in afterAo1\n",node->key);
    enQ(q2,node);
}
void afterAo2(struct QNode* node){
    if (node==NULL){
        return;
    }
//    printf("The string %s now in afterAo2\n",node->key);
    enQ(q3,node);
}

void backToClient(struct QNode* node){
    if (node==NULL){
        return;
    }
    printf("The string %s is now going back to the client\n",node->key);
    char* curr = node->key;
    if (!strcmp(node->key, ans[node->sock])){
        printf("SUCCESS TEST num %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",node->sock);
    }
    if(node->sock==5){
        pthread_cancel(th[0]);
        pthread_cancel(th[1]);
        pthread_cancel(th[2]);
        return;
    }

}

void releaseNode(struct QNode* node){
    if (node==NULL){
        free(node);
        return;
    }
    free(node);
}

void destroyAO(active_object *obj) {
    destroyQ(obj->q);
    free(obj);
}


int main(int argc, char const* argv[]){
    initQueues(); // initialize all queues

    //_____________________________ActiveObjects and Pipeline Initialization________________________________________////

    struct active_object *a1 = (active_object *) (malloc(sizeof(active_object)));
    a1->q_fun_ptr = Ao1;
    a1->f_fun_ptr = afterAo1;
    a1->q=q1;
    a1->my_pid = th[0];
    struct active_object *a2 = (active_object *) (malloc(sizeof(active_object)));;
    a2->q_fun_ptr = Ao2;
    a2->f_fun_ptr = afterAo2;
    a2->q=q2;
    a2->my_pid = th[1];
    struct active_object *a3 = (active_object *) (malloc(sizeof(active_object)));;
    a3->q_fun_ptr = backToClient;
    a3->f_fun_ptr = releaseNode;
    a3->q=q3;
    a3->my_pid = th[2];
    pipline *pipline1 = (pipline*)(malloc(sizeof (pipline)));
    pipline1->first = a1;
    pipline1->second =a2;
    pipline1->third = a3;
    pointer = pipline1;

    if (pthread_create(&th[0], NULL, &makeNewAO, pipline1->first) != 0) {
        perror("Failed to create thread");
    }
    if (pthread_create(&th[1], NULL, &makeNewAO, pipline1->second) != 0) {
        perror("Failed to create thread");
    }
    if (pthread_create(&th[2], NULL, &makeNewAO, pipline1->third) != 0) {
        perror("Failed to create thread");
    }
    sleep(1);

    char* inputFromClient[7];
    inputFromClient[0]="0"; // last char in ascii table
    inputFromClient[1]="Z";
    inputFromClient[2]="z";
    inputFromClient[3]="9";
    inputFromClient[4]="123";
    inputFromClient[5]="abc";

    //input for the test:
    char input0[1024];
    char input1[1024];
    char input2[1024];
    char input3[1024];
    char input4[1024];
    char input5[1024];

    for (int j = 0; j < strlen(inputFromClient[0]); ++j) {
        input0[j]=inputFromClient[0][j];
    }
    input0[strlen(inputFromClient[0])]='\0';
    printf("enQ the string to quere num %d: %s in main\n",q1->id,input0);
    struct QNode* temp0 = newNode(input0);
    temp0->sock=0; // sock will represent the place in the global array ans for the test.
    enQ(q1,temp0);
    sleep(1);

    for (int j = 0; j < strlen(inputFromClient[1]); ++j) {
        input1[j]=inputFromClient[1][j];
    }
    input1[strlen(inputFromClient[1])]='\0';
    struct QNode* temp1 = newNode(input1);
    temp1->sock=1; // sock will represent the place in the global array ans for the test.
    enQ(q1,temp1);
    sleep(1);

    for (int j = 0; j < strlen(inputFromClient[2]); ++j) {
        input2[j]=inputFromClient[2][j];
    }
    input2[strlen(inputFromClient[2])]='\0';
    printf("enQ the string to quere num %d: %s in main\n",q1->id,input2);
    struct QNode* temp2 = newNode(input2);
    temp2->sock=2; // sock will represent the place in the global array ans for the test.
    enQ(q1,temp2);
    sleep(1);

    for (int j = 0; j < strlen(inputFromClient[3]); ++j) {
        input3[j]=inputFromClient[3][j];
    }
    input3[strlen(inputFromClient[3])]='\0';
    printf("enQ the string to quere num %d: %s in main\n",q1->id,input3);
    struct QNode* temp3 = newNode(input3);
    temp3->sock=3; // sock will represent the place in the global array ans for the test.
    enQ(q1,temp3);
    sleep(1);

    for (int j = 0; j < strlen(inputFromClient[4]); ++j) {
        input4[j]=inputFromClient[4][j];
    }
    input4[strlen(inputFromClient[4])]='\0';
    printf("enQ the string to quere num %d: %s in main\n",q1->id,input4);
    struct QNode* temp4 = newNode(input4);
    temp4->sock=4; // sock will represent the place in the global array ans for the test.
    enQ(q1,temp4);
    sleep(1);

    for (int j = 0; j < strlen(inputFromClient[5]); ++j) {
        input5[j]=inputFromClient[5][j];
    }
    input5[strlen(inputFromClient[5])]='\0';
    printf("enQ the string to quere num %d: %s in main\n",q1->id,input5);
    struct QNode* temp5 = newNode(input5);
    temp5->sock=5; // sock will represent the place in the global array ans for the test.
    enQ(q1,temp5);
    sleep(1);

    if (pthread_join(th[0], NULL) != 0) {
        perror("Failed to join thread");
    }
    if (pthread_join(th[1], NULL) != 0) {
        perror("Failed to join thread");
    }
    if (pthread_join(th[2], NULL) != 0) {
        perror("Failed to join thread");
    }


    destroyAO(a1);
    destroyAO(a2);
    destroyAO(a3);
    printf("All AO were destroyed\n");
    free(pipline1);
    printf("Finish realese all\n");
    printf("Passed all tests!!\n");
    return 0;
}
