// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>

#define PORT	 6789
#define MAXLINE 1024


int main() {
	int sockfd;
	char buffer[MAXLINE];

	struct sockaddr_in servaddr, cliaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);
	
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0 )
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	
	int len, n;
	
	while (1) {
		len = sizeof(cliaddr); //len is value/resuslt

		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
					MSG_WAITALL, ( struct sockaddr *) &cliaddr,
					&len);
		buffer[n] = '\0';
		printf("From client : %s\n", buffer);
		// buffer = toupper(buffer); 
		for(int i=0;i<strlen(buffer);i++){
			buffer[i] = toupper(buffer[i]);
	    	}

		sendto(sockfd, (const char *)buffer, strlen(buffer),
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
			len);
	
	}
	
	
	return 0;
}
