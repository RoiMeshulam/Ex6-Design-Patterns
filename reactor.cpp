#include "reactor.hpp"

void reactorLoop(reactor *reactor){
    printf("run reactor loop\n");
    int new_fd;        // Newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    char buf[256];    // Buffer for client data
    char remoteIP[INET6_ADDRSTRLEN];
    for (;;) {
        printf("reactor have %d count\n",reactor->fd_count);
        int poll_count = poll(reactor->pfds, reactor->fd_count, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }
        // Run through the existing connections looking for data to read
        for (int i = 0; i < reactor->fd_count; i++) {
            // Check if someone's ready to read
            if (reactor->pfds[i].revents & POLLIN) { // We got one!!
                reactor->moreFuncs[i](reactor->pfds[i].fd); // active the right function
            } // END got ready-to-read from poll()
        } // END looping through file descriptors
    }
}


void* newReactor(preactor reactor){
    printf("Active the reactor thread\n");
    pthread_t new_thread;
    if (pthread_create(&new_thread, NULL, (void* (*)(void*))reactorLoop, reactor) != 0) {
        perror("Failed to create thread");
    }
    if (pthread_join(new_thread, NULL) != 0) {
        perror("Failed to join thread");
    }
    return reactor;
}

void InstallHandler(preactor reactor,void*(func)(void*), int file_des){
    // If we don't have room, add more space in the pfds array and the morefuncs array
    printf("I am in install handler\n");
    if (reactor->fd_count == reactor->fd_size) {
        reactor->fd_size *= 2; // Double it
        reactor->pfds= (pollfd*)realloc(reactor->pfds, sizeof(reactor->pfds)*(reactor->fd_size));
        reactor->moreFuncs = (void (**)(int))realloc (reactor->moreFuncs, sizeof(reactor->moreFuncs)*reactor->fd_size);
    }
    reactor->pfds[reactor->fd_count].fd=file_des;
    reactor->pfds[reactor->fd_count].events=POLLIN;
    reactor->moreFuncs[reactor->fd_count]=(functype)func;
    reactor->fd_count++;
}


void RemoveHandler(preactor reactor,int file_des){
    printf("i am removing handler\n");
    for (int i = 0; i < reactor->fd_count ; ++i) {
        if (reactor->pfds[i].fd==file_des){
            reactor->pfds[i]=reactor->pfds[reactor->fd_count - 1];
            reactor->moreFuncs[i]=reactor->moreFuncs[reactor->fd_count -1];
            reactor->fd_count--;
        }
    }
}