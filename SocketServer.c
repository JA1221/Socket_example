#include <stdio.h>
#include <string.h>
#include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <arpa/inet.h>


int main(){
    // 建立socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("Fail to create a socket.\n");
		return -1;
	}

    // socket 綁定
	struct sockaddr_in clientAddr, serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(4567);

    int err = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(err == -1) {
        printf("Bind failed.\n");
        return -1;
    }

    // 監聽連線
	listen(sockfd, 3);
    printf("Listening...\n");

	socklen_t addrlen = sizeof(clientAddr);
    char buffer[1024] = "Server have received your data.";
	char receiveMsg[1024];

	while(1){  // receive data
		int connfd = accept(sockfd, (struct sockaddr *) &clientAddr, &addrlen);
        if(connfd == -1) {
            printf("Accept failed.\n");
            return -1;
        }
        // char *client_ip = inet_ntoa(clientAddr.sin_addr);
        char *client_ip = inet_ntoa(clientAddr.sin_addr);
        int client_port = ntohs(clientAddr.sin_port);
        printf("Connect form:%s Port:%d\n", client_ip, client_port);
        
        while(recv(connfd, receiveMsg, 1024, 0) > 0){
            printf("Received data: %s \n", receiveMsg);
            // send(connfd, buffer, strlen(buffer)+1, 0);
        }
		
        printf("Socket closed.\n");
        close(connfd);
	}
	return 0;
}