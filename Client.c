#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <netinet/in.h> //structure for storing address information
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>

#define TRUE 1
const int MAX_CMD_SIZE = 1024;

int main() {
    int sockD = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port= htons(3490); // use some unused port number
    servAddr.sin_addr.s_addr = INADDR_ANY;

    int connectStatus = connect(sockD, (struct sockaddr*)&servAddr,sizeof(servAddr));

    if (connectStatus == -1) {
        printf("Error...\n");
        return 0;
    }
    //Accepting data from the server
    char newMSG[MAX_CMD_SIZE];
    fgets(newMSG,MAX_CMD_SIZE,stdin);
    newMSG[strlen(newMSG) - 1] = '\0';
    send(sockD, newMSG, strlen(newMSG), 0);
    char serverData[1024];
    int numOfBytes;
    numOfBytes = recv(sockD, serverData, 1024, 0);
    serverData[numOfBytes] = '\0';
    printf("%s\n",serverData);
    return 0;
}
