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
	fd_set master;    // master file descriptor list
    	fd_set read_fds;  // temp file descriptor list for select()
    	int fdmax;        // maximum file descriptor number

    	int listener;     // listening socket descriptor
    	int newfd;        // newly accept()ed socket descriptor
    	int addr_size;     // length of client addr
	struct sockaddr_in server_addr, client_addr;
	
	char buffer[MAX]; // buffer for client data
	int nbytes;
	
	int yes=1;        // for setsockopt() SO_REUSEADDR, below
    	int i, j, rv;
	
	FD_ZERO(&master);    // clear the master and temp sets
    	FD_ZERO(&read_fds);
    	
	// sget us a socket and bind it
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1) {
		printf("Socket creation failed...\n");
		exit(1);
	}
	else
		printf("Socket successfully created..\n");
	
	// lose the pesky "address already in use" error message
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
	bzero(&server_addr, sizeof(server_addr));
	
	// asign IP, PORT
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	
	// Binding newly created socket to given IP and verification
	if ((bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr))) != 0) {
		printf("Socket bind failed...\n");
		exit(2);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(listener, 5)) != 0) {
		printf("Listen failed...\n");
		exit(3);
	}
	else
		printf("Server listening..\n");
		
	// add the listener to the master set
    	FD_SET(listener, &master);
    	
    	// keep track of the biggest file descriptor
    	fdmax = listener; // so far, it's this one
	
	// main loop
	for(;;) {
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
		    perror("select");
		    exit(4);
		}
		// run through the existing connections looking for data to read
        	for(i = 0; i <= fdmax; i++) {
            	  if (FD_ISSET(i, &read_fds)) { // we got one!!
                    if (i == listener) {
                      // handle new connections
                      addr_size = sizeof(client_addr);
                      newfd = accept(listener, (struct sockaddr*)&client_addr, &addr_size);
                      if (newfd == -1){
                        perror("accept");
                      } else {
                          FD_SET(newfd, &master); // add to master set
                          if (newfd > fdmax) { // keep track of the max
                            fdmax = newfd;
                          }
                          printf("selectserver: new connection from %s on "
                            "socket %d\n",inet_ntoa(client_addr.sin_addr),newfd);
                      }
                    } else {
                        // handle data from a client
                        if ((nbytes = recv(i, buffer, sizeof buffer, 0)) <= 0) {
                          // got error or connection closed by client
                          if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                          } else {
                            perror("recv");
                          }
                          close(i); // bye!
                          FD_CLR(i, &master); // remove from master set   
                        } else {
                            // we got some data from a client
                            // print buffer which contains the client contents
                            printf("From client: %s", buffer);
                            // transform the characters into uppercase
                            for (int ii=0;ii<strlen(buffer);ii++){
                              buffer[ii] = toupper(buffer[ii]);
                            }
                            // if msg contains "Exit" then server exit and chat ended.
                            if (strncmp("EXIT", buffer, 4) == 0) {
                              printf("Client Exit...\n");
                              close(i); // bye!
                              FD_CLR(i, &master);  // remove from master set  
                            }
                            else{
                              // and send that buffer to client
                              if (send(i, buffer, sizeof(buffer), 0) == -1) {
                                perror("send");
                              } else {
                                printf("To client: %s", buffer);
                              }
                            }

                        }
                    } // end handle data from client
                  } // end got new incoming connection
                } // end looping through file descriptors
        } // end for(;;) 
		

	return 0;
}
