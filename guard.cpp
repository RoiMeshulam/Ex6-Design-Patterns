#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <pthread.h>

using namespace std;
void *glob_ptr;
pthread_mutex_t mutex_lock;

class Guard {
public:
    Guard() {
        if (pthread_mutex_init(&mutex_lock, NULL) != 0) {
            printf("mutex init failed\n");
        }
        else{
            pthread_mutex_lock(&mutex_lock);
            cout << "Guard created\n"<<endl;
        }
    }
    ~Guard() {
        pthread_mutex_unlock(&mutex_lock);
        cout << "guard destroyed\n"<<endl;
    }
};


void *change_pointer(void *p) {
    Guard g();
    glob_ptr = p;
    cout << "The global pointer was changed and now glob_ptr is: "<<*((string*)glob_ptr)<<endl;
    return NULL;
}

int main(int argc, char const *argv[]) {
    pthread_t th1 , th2;
    string ptr1 = "first_change";
    string ptr2 = "second_change";

    pthread_create(&th1, NULL, &change_pointer, &ptr1);
    pthread_create(&th2, NULL, &change_pointer, &ptr2);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    cout << "glob_ptr now is: " << *((string*)glob_ptr)<< endl;
    return 0;
}

