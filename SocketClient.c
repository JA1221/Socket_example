#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#ifdef _WIN32 // windows (-l wsock32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else // Linux
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#define UPDATE_INTERVAL 10 // s
// #define DOWNLOAD_TMP_FOLDER "/tmp/"
#define DOWNLOAD_TMP_FOLDER "C:\\Users\\robin\\Downloads\\"

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
    long fileSize = 0;

    while ((len = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        fileSize += len;

        if(fwrite(buffer, sizeof(char), len, fp) < len) {
            printf("Image Write Failed\n");
            return -1;
        }
        else
            memset(buffer, 0, sizeof(buffer));
    }
    
    if(len < 0 || fileSize == 0)
        return -1;
    else
        return 0;
}

int updateAPP(char *addr, int port, Update_Information *ui) {
    printf("< Update... >\n");

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

        char exePath[256] = "";
        char downloadPath[256] = "";
        char cmd[256] = "";

        strcpy(exePath, ui->list[i].exePath);
        strcat(exePath, ui->list[i].exeName);

        strcpy(downloadPath, DOWNLOAD_TMP_FOLDER);
        strcat(downloadPath, ui->list[i].exeName);

        // tmp
        // sprintf(cmd, "cp %s %s", exePath, downloadPath);
        sprintf(cmd, "copy %s %s > NUL", exePath, downloadPath);
        system(cmd);

        // open download path
        FILE *fp = fopen(downloadPath, "wb");
        
        // download APP
        if(recvFile(sockfd, fp) == -1) {
            printf("   Recv: Error\n", i+1);
            fclose(fp);

            // delete download file
            // sprintf(cmd, "rm -f %s", downloadPath);
            sprintf(cmd, "del -q %s", downloadPath);
            system(cmd);
        }else{
            // replace APP

            printf("   Recv: Success\n", i+1);
            fclose(fp);
            
            // kill process
            // sprintf(cmd, "killall %s 2> /dev/null", ui->list[i].exeName);
            // system(cmd);

            // mv app from downloadFolder to targetFolder
            // sprintf(cmd, "mv %s %s", downloadPath, exePath);
            sprintf(cmd, "move %s %s > NUL", downloadPath, exePath);
            system(cmd);

            // exe
            if(ui->list[i].run) {
                if(ui->list[i].run == 2)
                    sprintf(cmd, "%s &", exePath);
                system(exePath);
            }
        }
        
        // close connect
        closeSocket(sockfd);
        if(i < count-1)
            printf("\n");
    }

    printf("< End of update. >\n\n");
}

void printTime() {
    time_t now;
    time(&now);
    printf("%s", ctime(&now));
}

int main(int argc, char **argv) {
    // set IP & port
    char serverIP[15] = "140.115.52.115";
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

    while(1) {
        /* 
            Create Socket & Connect to server
            & return connect socket fd
        */
        printTime();
        int sockfd = connectSocket(serverIP, serverPort);
        if(sockfd == -1) {
            // exit(EXIT_FAILURE);
            sleep(UPDATE_INTERVAL);
            continue;
        }

        // Send a message to server
        char buffer[1024] = "OK";
        char receiveMsg[1024];

        // update list
        Update_Information *ui = (Update_Information *) malloc(sizeof(Update_Information));
        int recvN = recv(sockfd, (void*)ui, sizeof(Update_Information), 0);

        if(recvN > 0) 
            send(sockfd, buffer, strlen(buffer) + 1, 0); // "OK"
        else
            printf("Update Info recv ERROR\n");

        // Update APPs
        updateAPP(serverIP, serverPort, ui);
        free(ui);

        // close socket
        closeSocket(sockfd);


        // sleep
        sleep(UPDATE_INTERVAL);
    }

    // close winsock2
    #ifdef _WIN32
    WSACleanup();
    #endif

    return 0;
}