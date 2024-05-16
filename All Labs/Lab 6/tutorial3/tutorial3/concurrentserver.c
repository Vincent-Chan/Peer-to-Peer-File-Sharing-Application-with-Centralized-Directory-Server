#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

#define MAXLINE 1024
#define PORT 6789

/*
Process Terminated Child processes
*/
static void
sigchld_handler(int signo){
    pid_t PID;
    int status;

    do {
        PID = waitpid(-1, &status, WNOHANG);
    } while (PID != -1);

    /* Re-instate handler*/
    signal(SIGCHLD, sigchld_handler);
}

/*
This funcion reports the error and exits back to the shell
*/
static void
bailOut(const char *on_what) {
    if (errno != 0) {
        fputs(strerror(errno), stderr);
        fputs(": ", stderr);
    }
    fputs(on_what, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main (int argc, char **argv) {
    int z;
    struct sockaddr_in server_addr, client_addr;
    int server_socket = -1;
    int client_socket = -1;
    char buf[MAXLINE];
    int len_inet;
    pid_t PID;

    // Normally SIGCHLD is ignored by default. In this example, we register 
    // the function sgchld_handler() as the function called whenever the kernel 
    // send a SIGCHLD to the parent, i.e., whenever a child process terminates
    
    signal(SIGCHLD, sigchld_handler);

    // Create a TCP/IP server socket:
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        bailOut("socket(2)");
    
    memset(&server_addr, 0, sizeof(server_addr));
	
    // asign IP, PORT
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    
    // Bind the server address:
    z = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (z == -1)
        bailOut("bind(2)");
    
    // Make a listening socket:
    z = listen(server_socket, 10);
    if (z == -1)
        bailOut("listen(2)");

    // Start the server loop
    for (;;) {
        // Wait for a connect request from clients
        len_inet = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &len_inet);
        if (client_socket == -1)
            bailOut("accept(2)");
        
        // A client connection has been accepted and client_socket was created
        // Fork a new server process to service this client on that client_socket
        if ((PID = fork()) == -1) {
            // Failed to fork: Give up
            close(client_socket);
            continue;
        } else if (PID > 0) {
            // Parent process must continue to accept other clients so it does not need THIS client_socket
            close(client_socket);
            continue;
        }
        // Child process, we do not need the server socket and should close it.
        printf("Start communicating with client, ");
        printf("IP address is: %s, ", inet_ntoa(client_addr.sin_addr));
        printf("port is: %d\n", (int) ntohs(client_addr.sin_port));
        memset(&buf, 0, sizeof(buf));
        // Read the message from client and copy it in buffer
		recv(client_socket, buf, sizeof(buf), 0);
		// Print buffer which contains the client contents
		printf("From client: %s", buf);
		// Transform the characters into uppercase
		for (int i=0;i<strlen(buf);i++){
			buf[i] = toupper(buf[i]);
		}
		// And send that buffer to client
		send(client_socket, buf, sizeof(buf), 0);
		printf("To client: %s", buf);
        
        // Close the client socket
        close(client_socket);
        // Child process must exit:
        exit(0);
    }
    // Control never gets here
    return 0;
}
