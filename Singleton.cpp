#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <fstream>

using namespace std;

pthread_mutex_t mutex_th;

template<typename T>
class Singleton {
private:
    static Singleton *obj;
    T my_t;

    Singleton(T temp);
    mutex single_mtx;

public:
    static Singleton *Instance(T temp);
    void Destroy();
};

template<typename T>
Singleton<T> *Singleton<T>::obj = 0;

template<typename T>
Singleton<T> *Singleton<T>::Instance(T temp) {
    if (obj == 0) {
        pthread_mutex_lock(&mutex_th);
        obj = new Singleton(temp);
    }
    pthread_mutex_unlock(&mutex_th);
    return obj;
}

template<typename T>
Singleton<T>::Singleton(T temp) {
    single_mtx.lock();
    my_t = temp;
}

template<typename T>
void Singleton<T>::Destroy() {
    obj = 0;
    single_mtx.unlock();

}

int main() {
    FILE *fptr;
    Singleton<FILE *> *ptr1 = Singleton<FILE *>::Instance(fptr);
    Singleton<FILE *> *ptr2 = Singleton<FILE *>::Instance(fptr);
    std::cout << ptr1 << std::endl;
    std::cout << ptr2 << std::endl;
}
