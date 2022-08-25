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

#define MAX_FILE_PATH_SIZE 128
// #define MAX_BUFFER_SIZE 20000
#define MAX_BUFFER_SIZE (1 << 16)

int main(int argc, char **argv){
    // set port
    int serverPort = 4567;
    if(argc == 2)
        serverPort = atoi(argv[1]);
    printf("Listening Ports : %d\n\n", serverPort);


    //Winsows啟用socket
    #ifdef _WIN32
    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2,2), &wsadata) == SOCKET_ERROR) {
        printf("WSAStartup() fail\n");
        exit(1);
    }
    #endif


    // 建立socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.\n");
        exit(1);
    }

    // socket 綁定
    struct sockaddr_in clientAddr, serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);

    int err = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(err == -1) {
        printf("Bind failed.\n");
        exit(1);
    }

    // 監聽連線
    listen(sockfd, 3);
    printf("Listening...\n");
    socklen_t addrlen = sizeof(clientAddr);

    // open BS file

    const char bs_files[][MAX_FILE_PATH_SIZE] = {"LED.bit",
                                                "LED.bit.bin",
                                                "LED_v2.bit",
                                                "LED_v2.bit.bin"};
    const int files_num = sizeof(bs_files) / sizeof(bs_files[0]);

    char buffer[MAX_BUFFER_SIZE];
    char recv_buffer[MAX_BUFFER_SIZE];

    int file_index;
    for(int i = 0; i < files_num; i++){
        printf("%d) %s\n", i, bs_files[i]);
    }

    printf("Select file:");
    scanf("%d", &file_index);

    FILE *fp = fopen(bs_files[file_index], "rb");

    if(fp == NULL){
        printf("Image file %s not found", bs_files[file_index]);
        exit(1);
    }
    
    while(1){ 
        int connfd = accept(sockfd, (struct sockaddr *) &clientAddr, &addrlen);
        if(connfd == -1) {
            printf("Accept failed.\n");
            exit(1);
        }

        char *client_ip = inet_ntoa(clientAddr.sin_addr);
        int client_port = ntohs(clientAddr.sin_port);
        printf("Connect form:%s Port:%d\n", client_ip, client_port);
        
        // send image
        int len = 0;
        int sum = 0;

        while ((len = fread(buffer, sizeof(char), MAX_BUFFER_SIZE, fp)) > 0) {
            printf("read len: %d\n", len);
            sum += len;
            if (send(connfd, buffer, len, 0) < 0) { 
                printf("Send Image File: %s Failed\n", bs_files[file_index]);
                break;
            }
            printf("sum: %d\n", sum);
            memset(buffer, 0, MAX_BUFFER_SIZE); 
        }
        // seek back to beginning of file
        fseek(fp, 0, SEEK_SET);

        printf("Socket closed.\n");

        // close socket
        #ifdef _WIN32
        closesocket(connfd);
        #else
        close(connfd);
        #endif
    }
    fclose(fp);

    return 0;
}