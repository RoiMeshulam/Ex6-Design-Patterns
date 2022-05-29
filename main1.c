#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <pthread.h>
#include "Queue.h"
#define TRUE 1

/*
 * Global queues
 */
struct Queue* q1 = NULL;
struct Queue* q2 = NULL;
struct Queue* q3 = NULL;

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
    printf("NewAO is established\n");
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
    printf("The string is in AO1\n");
    char* str = node->key;
    printf("The string is %s\n",str);
    int len = strlen(node->key);
    for (int i = 0; i < len; i++){
        if (node->key[i] == 'z'){
            node->key[i] = 'a';
        }
        else if (node->key[i] =='Z'){
            node->key[i] = 'A';
        }
        else{
            node->key[i] += 1;
        }
    }
}

void Ao2(struct QNode* node){
    if (node==NULL){
        return;
    }
    printf("The string is in AO2\n");
    printf("The string is %s\n",node->key);
    int len = strlen(node->key);
    for (int i = 0; i < len; i++){
        if (node->key[i]>=65 && node->key[i] <=90){
            node->key[i] += 32;
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
    printf("Now in afterAo1\n");
    printf("The string is %s\n",node->key);
    enQ(q2,node);
}
void afterAo2(struct QNode* node){
    if (node==NULL){
        return;
    }
    printf("Now in afterAo2\n");
    printf("The string is %s\n",node->key);
    enQ(q3,node);
}

void backToClient(struct QNode* node){
    if (node==NULL){
        return;
    }
    printf("The string is %s\n",node->key);
    char* curr = node->key;
    send(node->sock,curr, strlen(curr),0);
    close(node->sock);
    printf("back to client.. \n");
}

void releaseNode(struct QNode* node){
    if (node==NULL){
        return;
    }
    printf("In check func\n");
    free(node);
}

void destroyAO(active_object *obj) {
    printf("try to destroy %d\n",obj->q->id);
    destroyQ(obj->q);
    printf("after destroyQ\n");
    free(obj);
    printf("destroy AO finished!!\n");
}

/*
 * This is the function we are giving to the new thread that receiving data from client
 */
void *newFunc(struct QNode *node){
    printf("enQ the string: %s\n",node->key);
    enQ(q1,node);
    printf("%d\n",q1->id);
}

int main(int argc, char const* argv[]){
    initQueues(); // initialize all queues
    pthread_t th[3];

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

    //_______________________________________ServerInitialization____________________________________________________//

    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    // define server address
    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(3490);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // bind socket to the specified IP and port
    bind(servSockD, (struct sockaddr*)&servAddr,sizeof(servAddr));

    // listen for connections
    listen(servSockD, 10);
    printf("listening..\n");

    //____________________________Listening to connections from clients_________________________________________//

    while (TRUE){
        // integer to hold client socket.
        int clientSocket = accept(servSockD, NULL, NULL);
        printf("holding client socket\n");
        pthread_t newThread;
        char clientData[1024];
        int numOfBytes;
        numOfBytes = recv(clientSocket, clientData, 1024, 0);
        clientData[numOfBytes] = '\0';
        printf("got data\n");
        printf("%s\n",clientData);
        if (!strcmp(clientData, "EXIT")){
             printf("EXIT was entered\n");
             close(clientSocket);
             break;
        }else{
            struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
            temp->key = clientData;
            temp->next=NULL;
            temp->sock = clientSocket;
            pthread_create(&newThread,NULL, newFunc,temp);
        }
    }
    printf("after while loop\n");
    sleep(5);
    destroyAO(a1);
    destroyAO(a2);
    destroyAO(a3);
    free(pipline1);
    close(servSockD);
    printf("Finish realese all\n");
    return 0;
}