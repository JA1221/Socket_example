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

typedef struct {
    char exeName[40];
    char exePath[80];
    int run;
} APP_Info;

typedef struct {
    APP_Info list[10];
	int list_cnt;
} Update_Information;

int connectSocket(char *addr, int port) {
    int sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if(sockfd == -1) {
        printf("Fail to create a socket.\n");
        return -1;
    }
    

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(addr);
    serverAddr.sin_port = htons(port);

    int err = connect(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
    if(err == -1) {
        printf("Connection error\n");
        return err;
    }

    return sockfd;
}

void closeSocket(int sockfd) {
    // printf("[-]Socket closed.\n");
    // close socket
    #ifdef _WIN32
    closesocket(sockfd);
    #else
    close(sockfd);
    #endif
}

int recvFile(int sockfd, FILE *fp) {
    char buffer[1024];
    int len;

    while ((len = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        if(fwrite(buffer, sizeof(char), len, fp) < len) {
            printf("Image Write Failed\n");
            return -1;
        }
        else
            memset(buffer, 0, sizeof(buffer));
    }
    
    if(len < 0)
        return -1;
    else
        return 0;
}

int updateAPP(char *addr, int port, Update_Information *ui) {
    char error[] = "Error";
    char success[] = "Success";

    printf("Update...\n");

    int count = ui->list_cnt;
    for(int i = 0; i < count; i++){
        // create connect
        int sockfd = connectSocket(addr, port);
            if(sockfd == -1)
        return -1;

        // APP info
        printf("[+]APP %d\n", i+1);
        printf("   Name: %s\n", ui->list[i].exeName);
        printf("   Path: %s\n", ui->list[i].exePath);
        // printf("Run:%d\n", ui->list[i].run);

        // APP path
        char path[256] = "";
        strcat(path, ui->list[i].exePath);
        strcat(path, ui->list[i].exeName);
        FILE *fp = fopen(path, "wb");
        
        // download APP
        if(recvFile(sockfd, fp) == -1){
            printf("   Recv: Error\n", i+1);
            send(sockfd, error, sizeof(error) + 1, 0);
        }
        else{
            printf("   Recv: Success\n", i+1);
            send(sockfd, success, sizeof(success) + 1, 0);
        }
        
        fclose(fp);
        
        // close connect
        closeSocket(sockfd);
        if(i < count-1)
            printf("\n");
    }

    printf("End of update.\n\n");
}


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
        exit(EXIT_FAILURE);
    }
    #endif

    /* 
        Create Socket & Connect to server
        & return connect socket fd
    */
    int sockfd = connectSocket(serverIP, serverPort);
    if(sockfd == -1)
        exit(EXIT_FAILURE);

    // Send a message to server
    char buffer[1024] = "OK";
    char receiveMsg[1024];

    // update list
    Update_Information *ui = (Update_Information *) malloc(sizeof(Update_Information));
    int recvN = recv(sockfd, (void*)ui, sizeof(Update_Information), 0);

    if(recvN > 0) 
        send(sockfd, buffer, strlen(buffer) + 1, 0); // "OK"
    else
        printf("recv ERROR\n");

    // Update APPs
    updateAPP(serverIP, serverPort, ui);

    // close socket
    closeSocket(sockfd);

    // close winsock2
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    free(ui);

    return 0;
}