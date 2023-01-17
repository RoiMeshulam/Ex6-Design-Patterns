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
}

typedef struct active_object {
    struct Queue *q;
    void* (*first_func_ptr)(void*);
    void* (*after_func_ptr)(void*);
    pthread_t my_th;
} active_object;

typedef struct pipline {
    active_object *first;
    active_object *second;
    active_object *third;
} pipline;


void makeNewAO(active_object *ao){
    printf("Making newAO for Queue %d\n",ao->q->id);
    newAO(ao->q,ao->first_func_ptr,ao->after_func_ptr);
}

void newAO(struct Queue *q, void (*first_func_ptr)(), void (*after_func_ptr)()) {
    printf("NewAO is established\n");
    while (1){
        if (q->status==-1){
            break;
        }
        struct QNode* temp = deQ(q);
        (*first_func_ptr)(temp);
        (*after_func_ptr)(temp);
    }

}

//__________________________________________FunctionsForActiveObjects________________________________________________//

void Ao1(struct QNode* node){
    if (node==NULL){
        return;
    }
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
            node->key[i] += 1;
        }
    }
}

void Ao2(struct QNode* node){
    if (node==NULL){
        return;
    }
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
    enQ(q2,node);
}
void afterAo2(struct QNode* node){
    if (node==NULL){
        return;
    }
    enQ(q3,node);
}

void backToClient(struct QNode* node){
    if (node==NULL){
        return;
    }
    printf("The string %s is now going back to the client\n",node->key);
    char* curr = node->key;
    send(node->sock,curr, strlen(curr),0);
    close(node->sock);
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

/*
 * This is the function we are giving to the new thread that receiving data from client
 */
void *newFunc(struct QNode *node){
    printf("enQ the string: %s\n",node->key);
    enQ(q1,node);
}

int main(int argc, char const* argv[]){
    initQueues(); // initialize all queues
    pthread_t th[3];

    //_____________________________ActiveObjects and Pipeline Initialization________________________________________////

    struct active_object *a1 = (active_object *) (malloc(sizeof(active_object)));
    a1->first_func_ptr = Ao1;
    a1->after_func_ptr = afterAo1;
    a1->q=q1;
    a1->my_th = th[0];
    struct active_object *a2 = (active_object *) (malloc(sizeof(active_object)));;
    a2->first_func_ptr = Ao2;
    a2->after_func_ptr = afterAo2;
    a2->q=q2;
    a2->my_th = th[1];
    struct active_object *a3 = (active_object *) (malloc(sizeof(active_object)));;
    a3->first_func_ptr = backToClient;
    a3->after_func_ptr = releaseNode;
    a3->q=q3;
    a3->my_th = th[2];
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
        printf("A מew connection has been established\n");
        pthread_t newThread;
        char clientData[1024];
        int numOfBytes;
        numOfBytes = recv(clientSocket, clientData, 1024, 0);
        clientData[numOfBytes] = '\0';
        printf("Server got data from client\n");
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
    sleep(2);
    destroyAO(a1);
    destroyAO(a2);
    destroyAO(a3);
    printf("All AO were destroyed\n");
    free(pipline1);
    close(servSockD);
    printf("The server closed\n");
    return 0;
}