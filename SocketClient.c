#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32 // windows (-l wsock32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else // Linux
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

int main(int argc, char **argv) {
    // set IP & port
    char serverIP[15] = "127.0.0.1";
    int serverPort = 4567;

    if(argc == 3) {
        strcpy(serverIP, argv[1]);
        serverPort = atoi(argv[2]);
    }
    printf("Server IP   : %s\n", serverIP);
    printf("Server Port : %d\n\n", serverPort);

    //Winsows啟用socket
    #ifdef _WIN32
    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2,2), &wsadata) == SOCKET_ERROR) {
        printf("WSAStartup() fail\n");
        exit(1);
    }
    #endif

    // 建立socket
    int sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if(sockfd == -1) {
        printf("Fail to create a socket.\n");
        exit(1);
    }

    // socket的連線
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(serverPort);
    
    int err = connect(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
    if(err == -1) {
        printf("Connection error\n");
        exit(1);
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
    
    // close socket
    #ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
    #else
    close(sockfd);
    #endif
    
    return 0;
}