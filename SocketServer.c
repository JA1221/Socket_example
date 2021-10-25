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

#define APP_LIST_FOLDER ".\\app\\list2.txt"
#define APP_FOLDER ".\\app\\"

typedef struct {
    char exeName[40];
    char exePath[80];
    int run;
} APP_Info;

typedef struct {
    APP_Info list[10];
	int list_cnt;
} Update_Information;

int bindSocket(int sockfd, char *addr, int port) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(addr);
    serverAddr.sin_port = htons(port);
    
    int err = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    return err;
}

int acceptSocket(int sockfd) {
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    int err = accept(sockfd, (struct sockaddr *) &clientAddr, &addrlen);

    if(err != -1){
        char *client_ip = inet_ntoa(clientAddr.sin_addr);
        int client_port = ntohs(clientAddr.sin_port);
        printf("[+]Connect form:%s Port:%d\n", client_ip, client_port);
    }

    return err;
}

void closeSocket(int sockfd) {
    printf("[-]Socket closed.\n");
    // close socket
    #ifdef _WIN32
    closesocket(sockfd);
    #else
    close(sockfd);
    #endif
}

int sendFile(int sockfd, FILE *fp) {
    char buffer[1024];
    int len;

    while ((len = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0) {
        if (send(sockfd, buffer, len, 0) < 0) { 
            printf("Send File Failed\n");
            return -1;
        }
        memset(buffer, 0, sizeof(buffer)); 
    }

    return 0;
}

void read_UI_file(Update_Information *ui) {
    char contents[1024];
    FILE* fp = fopen(APP_LIST_FOLDER, "r");

    if (!fp) {
        printf("List not found!!\n");
        exit(EXIT_FAILURE);
    }

    // list count
    fscanf(fp, "%s", contents);
    int count = atoi(contents);
    ui->list_cnt = count;

    for(int i = 0; i < count; i++) {
        
        fscanf(fp, "%s", contents);
        strcpy(ui->list[i].exeName, contents);
        
        fscanf(fp, "%s", contents);
        strcpy(ui->list[i].exePath, contents);
        
        fscanf(fp, "%s", contents);
        ui->list[i].run = atoi(contents);
    }

    fclose(fp);
}

int uploadList(int sockfd, Update_Information *ui) {
    char receiveMsg[128];

    // send list
    int sendNum = send(sockfd, (void*)ui, sizeof(*ui), 0);
    printf("Send %d Bytes\n", sendNum);
    recv(sockfd, receiveMsg, sizeof(receiveMsg), 0);
    printf("Client recv update list: %s\n\n", receiveMsg);

    return (sendNum > 0) ? 0 : -1;
}

void uploadAPP(int sockfd, Update_Information *ui, char *appFolder) {
    char buf[128];

    printf("< Update... >\n");

    int count = ui->list_cnt;
    for(int i = 0; i < count; i++) {
        // connect Client
        int connfd = acceptSocket(sockfd);
        
        char path[256] = "";
        strcat(path, appFolder);
        strcat(path, ui->list[i].exeName);

        FILE *fp = fopen(path, "rb");
        if(fp == NULL){
            printf("File[%d] open failed!\n", i+1);
            closeSocket(connfd);
            continue;
        }

        if(sendFile(connfd, fp) == -1)
            printf("   Send APP[%d]: ERROR.\n", i+1);
        else
            printf("   Send APP[%d]: Success.\n", i+1);

        fclose(fp);

        closeSocket(connfd);
        if(i < count-1)
            printf("\n");
    }

    printf("End of update.\n\n");
}

int main(int argc, char **argv){
    char buffer[1024] = "Server have received your data.";
    char receiveMsg[1024];

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

    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.\n");
        exit(1);
    }

    // bind
    int err = bindSocket(sockfd, "0.0.0.0", serverPort);
    if(err == -1) {
        printf("Bind failed.\n");
        exit(1);
    }

    // list
    listen(sockfd, 3);
    printf("Listening...\n");

    while (1)
    {
        // accept Client connection
        int connfd = acceptSocket(sockfd);
        
        // send list to client
        Update_Information ui;
        read_UI_file(&ui);
        uploadList(connfd, &ui);
        
        // upload APPs
        uploadAPP(sockfd, &ui, APP_FOLDER);
        // close socket
        closeSocket(connfd);
    }

    return 0;
}