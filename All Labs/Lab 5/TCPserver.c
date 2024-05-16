#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX 1024
#define PORT 6789

int main(){
	int server_socket, client_socket, addr_size;
	struct sockaddr_in server_addr, client_addr;
	char buffer[MAX];
	
	// socket create and verification
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	
	bzero(&server_addr, sizeof(server_addr));
	
	// asign IP, PORT
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	
	// Binding newly created socket to given IP and verification
	if ((bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) != 0) {
		printf("Socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(server_socket, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	
	while (1) {
		addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
		if (client_socket < 0){
			printf("Server accept failed...\n");
			exit(0);
		}
		else
			printf("Server accept the client...\n");
    		
    		
    		// chat with client
		bzero(buffer, MAX);
		
		// read the message from client and copy it in buffer
		recv(client_socket, buffer, sizeof(buffer), 0);
		// print buffer which contains the client contents
		printf("From client: %s", buffer);
		// transform the characters into uppercase
		for (int i=0;i<strlen(buffer);i++){
			buffer[i] = toupper(buffer[i]);
		}
		
		// and send that buffer to client
		send(client_socket, buffer, sizeof(buffer), 0);
		printf("To client: %s", buffer);
		
		close(client_socket);
    		printf("Client disconnected.\n\n");
		
	
	}

	return 0;
}
