#include <stdio.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h> // close function

int main() {
    // 建立socket
    int sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if(sockfd == -1) {
        printf("Fail to create a socket.\n");
        return -1;
    }

    // socket的連線
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(4567);
    
    int err = connect(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
    if(err == -1) {
        printf("Connection error\n");
        return -1;
    }

    // Send a message to server
    char buffer[1024];
    char receiveMsg[1024];

    while(strcmp(buffer, "q\n")) {
        printf("Send to server:");
        fgets(buffer, 1024, stdin);

        send(sockfd, buffer, strlen(buffer)+1, 0);
        // recv(sockfd, receiveMsg, sizeof(receiveMsg), 0);
        // printf("Received data: %s \n", receiveMsg);
    }
    printf("close Socket\n");
    close(sockfd);
    return 0;
}