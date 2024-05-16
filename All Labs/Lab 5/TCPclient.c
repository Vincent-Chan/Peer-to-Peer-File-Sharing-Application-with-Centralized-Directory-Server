#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 1024
#define PORT 6789

int main(){
	int sockfd, n;
	struct sockaddr_in server_addr, client_addr;
	char buffer[MAX];
	
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created...\n");
		
	bzero(&server_addr, sizeof(server_addr));
	
	// assign IP, PORT
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(PORT);
	
	// connect the client socket to the server socket
	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the server...\n");
		
	// chat with the server
	bzero(buffer, sizeof(buffer));
	printf("Enter the string: ");
	n = 0;
	while ((buffer[n++] = getchar()) != '\n')
		;
	send(sockfd, buffer, strlen(buffer), 0);
	
	bzero(buffer, sizeof(buffer));
	recv(sockfd, buffer, sizeof(buffer), 0);
	printf("From Server : %s", buffer);

	// close the socket
	close(sockfd);
	return 0;
}

