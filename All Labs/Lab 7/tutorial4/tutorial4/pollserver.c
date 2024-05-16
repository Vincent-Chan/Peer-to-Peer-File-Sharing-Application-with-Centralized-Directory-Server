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
#include <poll.h>

#define MAX 1024
#define PORT 6789 // Prot we're listening on

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

int main(){
	int listener;     // Listening socket descriptor
	int client_socket, addr_size;
	struct sockaddr_in server_addr, client_addr;
	// struct timeval tv;
	char buffer[MAX];
	
	// Start off with room for 5 connections
	// (We'll realloc as necessary)
	int fd_count = 0;
	int fd_size = 5;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);
    	
	// socket create and verification
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1) {
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
	if ((bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr))) != 0) {
		printf("Socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(listener, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	
	// Add the listener to set
    	pfds[0].fd = listener;
    	pfds[0].events = POLLIN; // Report ready to read on incoming connection

    	fd_count = 1; // For the listener

	// Main loop
	for(;;){
		int poll_count = poll(pfds, fd_count, -1);

        	if (poll_count == -1) {
            		perror("poll");
            		exit(1);
        	}
        	
        	// Run through the existing connections looking for data to read
        	for(int i = 0; i < fd_count; i++) {

            	// Check if someone's ready to read
            	if (pfds[i].revents & POLLIN) { // We got one!!

                	if (pfds[i].fd == listener) {
            		// If listener is ready to read, handle new connection
			addr_size = sizeof(client_addr);
			client_socket = accept(listener, (struct sockaddr*)&client_addr, &addr_size);
			
			if (client_socket == -1) {
                	perror("accept");
            		} else {
                	add_to_pfds(&pfds, client_socket, &fd_count, &fd_size);
                	printf("pollserver: new connection from %s on socket %d\n",inet_ntoa(client_addr.sin_addr),client_socket);
            		} 
            		
                    	} else {
                    	// If not the listener, we're just a regular client
                    	bzero(buffer, MAX);
                    	
                    	int nbytes = recv(pfds[i].fd, buffer, sizeof buffer, 0);

                    	int sender_fd = pfds[i].fd;

                    	if (nbytes <= 0) {
                        // Got error or connection closed by client
                        if (nbytes == 0) {
                            // Connection closed
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        } else {
                            perror("recv");
                        }

                        close(pfds[i].fd); // Bye!

                        del_from_pfds(pfds, i, &fd_count);
                    	} 
                    	else {
                        // We got some good data from a client\
                        // print buffer which contains the client contents
                        printf("From client: %s", buffer);
                        // transform the characters into uppercase
                        for (int ii=0;ii<strlen(buffer);ii++){
                        buffer[ii] = toupper(buffer[ii]);
                        }
                        // if msg contains "Exit" then server exit and chat ended.
                        if (strncmp("EXIT", buffer, 4) == 0) {
                        printf("Client Exit...\n");
                        close(pfds[i].fd);
                        del_from_pfds(pfds, i, &fd_count);
                        }
                        else{
                        int dest_fd = sender_fd;
                        // and send that buffer to client
                        send(dest_fd, buffer, sizeof(buffer), 0);
                        printf("To client: %s", buffer);
                        }

			}  
		}// end handle data from client
		} // end got ready-to-read from poll()
		} // end looping through file descriptors
	} // end for(;;)
	return 0;
}
