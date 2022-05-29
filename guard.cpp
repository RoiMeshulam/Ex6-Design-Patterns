#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <mutex>

using namespace std;
void *glob_ptr;
pthread_mutex_t mutex_lock;

class Guard {
public:
    Guard() {
        if (pthread_mutex_init(&mutex_lock, NULL) != 0) {
            printf("mutex init has failed\n");
        }
        else{
            pthread_mutex_lock(&mutex_lock);
            cout << "guard started\n"<<endl;
        }
    }
    ~Guard() {
        pthread_mutex_unlock(&mutex_lock);
        cout << "guard finished\n"<<endl;
    }
};


void *change_pointer(void *p) {
    Guard g();
    glob_ptr = p;
    cout << "The global pointer was changed\n";
    return NULL;
}

int main(int argc, char const *argv[]) {
    pthread_t t1 , t2;
    string ptr1 = "change1";
    string ptr2 = "change2";
    pthread_create(&t1, NULL, &change_pointer, &ptr1);
    pthread_create(&t2, NULL, &change_pointer, &ptr2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    cout << *((string*)glob_ptr)<< endl;
    return 0;
}

