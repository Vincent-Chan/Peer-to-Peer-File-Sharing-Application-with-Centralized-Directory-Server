#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>

// for BONUS part
#include <sys/time.h>


/***** BONUS part START *****/

// define the maximum number of query result can be stored as 128
#define MAX_NO_QUERY 128

/***** BONUS part END *****/


#define SERVER "127.0.0.1"
#define SERVER_PORT 5000

#define MAXMSG 1400
#define OP_SIZE 20

#define SEQ0 0
#define SEQ1 1

#define REGISTER "REGISTER"
#define UPDATE "UPDATE"
#define QUERY "QUERY"
#define RESPONSE "RESPONSE"
#define FINISH "FINISH"
#define ACK "ACK"
#define GET "GET"
#define EXIT "EXIT"

#define TIMEOUT 500000		/* 1000 ms */


/* This structure can be used to pass arguments */
struct ip_port {
	unsigned int ip;
	unsigned short port;
};


int set_timeout(int sockfd, int usec) {
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usec; /* 100 ms */
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
			(struct timeval *)&tv, sizeof(struct timeval));
	if (ret == SO_ERROR) {
		return -1;
	}
	return 0;
}


int unset_timeout(int sockfd) {
	return set_timeout(sockfd, 0);
}


int rdt3_send(int sockfd, struct sockaddr_in servaddr, char ack_num, char *buffer, unsigned len) {
	int waiting = 1;
	char noack_num = ack_num;
	struct sockaddr_in recv_addr;
	unsigned int recv_len;
	char recv_buf[MAXMSG];

	char seq;
	char op[OP_SIZE];
	char remain[MAXMSG];

	sendto(sockfd, (const char *)buffer, len,
		0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

	set_timeout(sockfd, TIMEOUT);

	int parse_idx = 0;

	while (waiting) {

		/***********************************************
		 * You should receive and parse the response here.
		 * Then, you should check whether it is the ACK
		 * packet you are waiting for by comparing the
		 * sequence number and checking packet type.
		 * Waiting until receiving the right ACK.
		 *
		 * START YOUR CODE HERE
		 **********************************************/

		// This part also require double checking, easy to implement wrong

		// reset parse_idx to 0 for every iteration in while loop
		parse_idx = 0;

		// fill recv_buf with 0 to clean up the things inside
		bzero(recv_buf, MAXMSG);

		// fill op with 0 to clean up the things inside
		bzero(op, OP_SIZE) ;

		// fill remian with 0 to clean up the things inside
		bzero(remain, MAXMSG) ;


		// receive the response from the server
		// (Not sure about whether the flag should be set to 0 or MSG_WAITALL)
		unsigned int recv_addr_len = sizeof(recv_addr) ;

		int received_length = 0 ;

		received_length = recvfrom(sockfd, &recv_buf, MAXMSG,
							0, ( struct sockaddr *) &recv_addr, &recv_addr_len) ;

		// if there is an error when receiving response,
		// or if we receive nothing, 
		// resend it and set the timeout
		if (received_length <= 0)
		{
			sendto(sockfd, (const char *)buffer, len,
				   0, (const struct sockaddr *) &servaddr, sizeof(servaddr)) ;
			
			set_timeout(sockfd, TIMEOUT) ;

			// printf("Some error occurs when receiving response from server!") ;

			continue ;
		}


		// check whether it is the ACK packet you are waiting for 
		// by comparing the sequence number and checking packet type
		// if it is ACK packet and the sequence number equals waiting ack num (noack_num),
		// this means we receive the right ACK
		// we need to set waiting to 0 in such situation

		// ACK packet: SEQ SPACE "ACK"
		// 1, 1, 3
		
		if (received_length == 5)
		{
			// get SEQ
			memcpy(&seq, recv_buf, sizeof(seq)) ;
			parse_idx += sizeof(seq) ;

			// we skip the SPACE
			parse_idx++ ;

			// get ACK
			memcpy(&op, recv_buf + parse_idx, sizeof(ACK)) ;
			parse_idx += sizeof(ACK) ;

			if (seq == noack_num && strncmp(op, ACK, sizeof(ACK)) == 0)
			{
				waiting = 0 ;
			}
		}

		
		/***********************************************
		 * END OF YOUR CODE
		 **********************************************/

		bzero(recv_buf, MAXMSG);
	}

	unset_timeout(sockfd);

	return 0;
}


/* Send query to the server */
int send_register(int sockfd, struct sockaddr_in servaddr, unsigned int ip, unsigned short port) {
	char buffer[MAXMSG];
	
	bzero(buffer, MAXMSG);

	char seq = SEQ1;

	/* Compose send buffer: REGISTER IP Port */
	int total_len = 0;

	memcpy(buffer, &seq, sizeof(seq));
	total_len ++; /* add a seq */

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, REGISTER, strlen(REGISTER));
	total_len += strlen(REGISTER);

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, &ip, sizeof(ip));
	total_len += sizeof(ip);

	memcpy(buffer + total_len, &port, sizeof(port));
	total_len += sizeof(port);

	buffer[total_len] = '\0';

	rdt3_send(sockfd, servaddr, seq, buffer, total_len);

	printf("REGISTER finished !\n");
	
	return 0;
}


/* Send query to the server */
int send_update(int sockfd, struct sockaddr_in servaddr, unsigned int ip, unsigned short port, unsigned file_map) {
	char buffer[MAXMSG];
	
	bzero(buffer, MAXMSG);

	char seq = SEQ1;

	/***********************************************
	 * You should follw the send_register pattern to
	 * implement a function to send UPDATE message.
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	/* Compose send buffer: SEQ SPACE "UPDATE" SPACE IP Port Bitmap */
	int total_len = 0 ;

	// add a SEQ of length 1
	memcpy(buffer, &seq, sizeof(seq)) ;
	total_len++ ;

	// add a SPACE of length 1
	buffer[total_len] = ' ' ;
	total_len++ ;

	// add an "UPDATE" of length 6
	memcpy(buffer + total_len, UPDATE, strlen(UPDATE)) ;
	total_len += strlen(UPDATE) ;

	// add a SPACE of length 1
	buffer[total_len] = ' ' ;
	total_len++ ;

	// add an IP of length 4
	memcpy(buffer + total_len, &ip, sizeof(ip)) ;
	total_len += sizeof(ip) ;

	// add a Port of length 2
	memcpy(buffer + total_len, &port, sizeof(port)) ;
	total_len += sizeof(port) ;

	// add a bitmap of length 4
	memcpy(buffer + total_len, &file_map, sizeof(file_map)) ;
	total_len += sizeof(file_map) ;

	buffer[total_len] = '\0' ;

	rdt3_send(sockfd, servaddr, seq, buffer, total_len) ;

	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/
	
	printf("UPDATE finished !\n");

	return 0;
}


/***** BONUS part START *****/

// add 1 more input parameter for this function
// query_record: struct array of ip_port* to store the query records
//
// change the return value to number of query records received

int receive_query(int sockfd, struct sockaddr_in servaddr, struct ip_port* query_record) {
	
	/***** BONUS part START *****/

	// number of query record received
	int no_query_received = 0 ;

	/***** BONUS part END *****/

	struct sockaddr_in recv_addr;
	unsigned int recv_len;
	char buffer[MAXMSG];

	int n = 0;
	unsigned parse_idx = 0;
	char seq;

	char send_buf[MAXMSG];
	unsigned send_idx = 0;

	unset_timeout(sockfd);

	printf("Receiving query ...\n");

	char unfinished = 1;
	while (unfinished) {

		n = recvfrom(sockfd, (char *)buffer, MAXMSG,
				MSG_WAITALL, ( struct sockaddr *) &recv_addr, &recv_len);
		if (n < 0) {
			continue;
		}

		buffer[n] = '\0';

		seq = buffer[0];
		parse_idx += 2; /* skip seq and blank */

		/* If receive FINISH signal, stop receiving. */
		if (strncmp(buffer + parse_idx, FINISH, strlen(FINISH)) == 0) {
			unfinished = 0;

		/* Receive RESPONSE */
		} else if (strncmp(buffer + parse_idx, RESPONSE, strlen(RESPONSE)) == 0) {
			// Receive and parse packet
			unsigned int ip;
			unsigned short port;

			parse_idx += strlen(RESPONSE);
			parse_idx ++; /*skip blank */

			memcpy(&ip, buffer + parse_idx, sizeof(ip));
			parse_idx += sizeof(ip);

			memcpy(&port, buffer + parse_idx, sizeof(port));
			parse_idx += sizeof(port);

			struct in_addr addr;
			addr.s_addr = ip;
			char *ip_str = inet_ntoa(addr);


			/***** BONUS part START *****/

			// output the sequence number
			printf("BONUS part: sequence number is  %d _ ", no_query_received) ;

			query_record[no_query_received].ip = ip ;
			query_record[no_query_received].port = port ;

			++no_query_received ;

			/***** BONUS part END *****/


			printf("%s : %d\n", ip_str, port);

		} else {
			printf("Unknown operation: %s\n", buffer + 2);
			goto nooperation;
		}


		// Compose and send ACK packet
		memcpy(send_buf, &seq, sizeof(seq));
		send_idx += 2; /* seq and blank */

		memcpy(send_buf + send_idx, ACK, strlen(ACK));
		send_idx += strlen(ACK);

		int res = sendto(sockfd, (const char *)send_buf, send_idx,
				0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

		if (res < 0) { 
		}

	nooperation:
		bzero(buffer, MAXMSG);
		bzero(send_buf, MAXMSG);
		parse_idx = 0;
		send_idx = 0;
	}

	// return 0;


	/***** BONUS part START *****/

	// return number of query records received
	return no_query_received ;

	/***** BONUS part END *****/


}

/***** BONUS part END *****/


/***** BONUS part START *****/

// add 1 more input parameter for this function
// query_record: struct array of ip_port* to store the query records
//
// change the return value to number of query records

/* Send query to the server */
int send_query(int sockfd, struct sockaddr_in servaddr, char *filename, int len, struct ip_port* query_record) {
	char buffer[MAXMSG];

	bzero(buffer, sizeof(buffer));

	char seq = SEQ1;

	/* Compose send buffer: REGISTER IP Port */
	int total_len = 0;

	memcpy(buffer, &seq, sizeof(seq));
	total_len ++; /* add a seq */

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, QUERY, strlen(QUERY));
	total_len += strlen(QUERY);

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, filename, len);
	total_len += len;

	buffer[total_len] = '\0';

	rdt3_send(sockfd, servaddr, seq, buffer, total_len);

	printf("QUERY finished !\n");

	/*sleep(1); [> begin to receive queried messages <]*/
	
	// receive_query(sockfd, servaddr);

	// return 0;


	/***** BONUS part START *****/

	// initialize a variable to store the number of query records
	int no_query = 0 ;
	
	// store the number of query records
	no_query = receive_query(sockfd, servaddr, query_record) ;

	// return the number of query records
	return no_query ;

	/***** BONUS part END *****/


}

/***** BONUS part END *****/


unsigned int get_file_map() {
	DIR *dir;
    struct dirent *entry;
	unsigned int file_map = 0U;

    // Open the current directory
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // Enumerate files in the directory
    while ((entry = readdir(dir)) != NULL) {
		unsigned int bit = (1U << 31);

		char file_idx = (entry->d_name[0] - '0') * 10 + (entry->d_name[1] - '0');
		if (file_idx < 0 || file_idx > 31) {
			continue;
		}


		file_map |= (bit >> file_idx);
    }

    // Close the directory
    closedir(dir);
	return file_map;
}


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


void* p2p_server(void* arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAXMSG] = {0};
    FILE *fp;
    ssize_t bytes_read;

	struct ip_port *ip_port_info = (struct ip_port *) arg;

	int fd_count = 0;
	int fd_size = 100; /* at most 100 sockets */
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

    // Bind socket to address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(ip_port_info->port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("listen fails to start");
        exit(EXIT_FAILURE);
    }

	printf("p2p server is listening ...\n");

	/***********************************************
	 * Initialize the pfds here.
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	// This part also require double checking, easy to implement wrong

	// add server_fd to pollfd set
	// and report that server_fd is ready to read on incoming connection
	// and add fd_count by 1
	
	// add server_fd to pollfd set
	// report that server_fd is ready to read on incoming connection
	pfds[0].fd = server_fd ;
	pfds[0].events = POLLIN ;

	// set fd_count to 1
	fd_count = 1 ;

	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/

	while (1) {
		int poll_count = poll(pfds, fd_count, -1);

		if (poll_count == -1) {
			perror("poll error");
			exit(1);
		}

		// Run through the existing connections looking for data to read
		for(int i = 0; i < fd_count; i++) {
			// Check if someone's ready to read
			if (pfds[i].revents & POLLIN) {
				if (pfds[i].fd == server_fd) { /* the server receives a connection request */
					/***********************************************
					 * Add your code here to receive a new connection
					 *
					 * START YOUR CODE HERE
					 **********************************************/

					// This part also require double checking, easy to implement wrong

					new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen) ;

					if (new_socket == -1)
					{
						perror("new_socket accept error") ;
						exit(EXIT_FAILURE) ;
					}
					else
					{
						add_to_pfds(&pfds, new_socket, &fd_count, &fd_size) ;
					}

					/***********************************************
					 * END OF YOUR CODE
					 **********************************************/


				} else { /* the client receive a message */

					new_socket = pfds[i].fd;

					// Receive file name from client
					if (recv(new_socket, buffer, MAXMSG, 0) < 0) {
						perror("recv");
						exit(EXIT_FAILURE);
					}


					// Open requested file
					fp = fopen(buffer, "rb");
					if (fp == NULL) {
						perror("fopen file error");
						exit(EXIT_FAILURE);
					}

					bzero(buffer, MAXMSG);

					long file_size;
					// Get the file size
					fseek(fp, 0, SEEK_END);
					file_size = ftell(fp);
					fseek(fp, 0, SEEK_SET);

					/***********************************************
					 * Refer to the description, send the file length
					 * and file content to the p2p client.
					 *
					 * START YOUR CODE HERE
					 **********************************************/

					// This part also require double checking, easy to implement wrong

					// send the file length
					// if we have error when sending file length to the client
					// call perror and exit(EXIT_FAILURE)
					if (send(new_socket, &file_size, sizeof(file_size), 0) < 0)
					{
						perror("send file size error") ;
						exit(EXIT_FAILURE) ;
					}

					// int feof( std::FILE* stream );
					//
					// stream: the file stream to check
					//
					// Return value:
					// Nonzero value if the end of the stream has been reached, otherwise ​0​.

					// use a while loop to read all the file content
					while (feof(fp) == 0)
					{
						// fill buffer with 0 to clean the things inside
						bzero(buffer, MAXMSG) ;

						// size_t fread(void *restrict buffer, 
						//				size_t size, 
						//				size_t count, 
						//				FILE *restrict stream );
						//
						// buffer: pointer to the array where the read objects are stored
						// size: size of each object in bytes
						// count: the number of the objects to be read
						// stream: the stream to read
						//
						// Return value:
						// Number of objects read successfully, which may be less than count if an error or end-of-file condition occurs.
						// If size or count is zero, fread returns zero and performs no other action.
						// fread does not distinguish between end-of-file and error, and callers must use feof and ferror to determine which occurred.

						// read the file content
						bytes_read = fread(buffer, sizeof(char), MAXMSG, fp) ;

						// int ferror( FILE *stream );
						//
						// stream: the file stream to check
						//
						// Return value:
						// Nonzero value if the file stream has errors occurred, ​0​ otherwise
						
						// if the file stream has error occur
						// call perror and exit(EXIT_FAILURE)
						if (ferror(fp) != 0)
						{
							perror("ferror file stream error") ;
							exit(EXIT_FAILURE) ;
						}

						// send the read content data to the client
						// if there is any error occur
						// call perror and exit(EXIT_FAILURE)
						if (send(new_socket, buffer, bytes_read, 0) < 0)
						{
							perror("send file to client error") ;
							exit(EXIT_FAILURE) ;
						}

					}

					/***********************************************
					 * END OF YOUR CODE
					 **********************************************/

					fclose(fp);
					close(new_socket);
					del_from_pfds(pfds, i, &fd_count);

					bzero(buffer, MAXMSG);


				}
			}
		}
	}

    close(server_fd);
	pthread_exit(NULL);
}


int p2p_client(unsigned int ip, unsigned short port, char *file_name) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAXMSG] = {0};
    FILE *fp;
    ssize_t bytes_read;

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fails to create");
        return -1;
    }

    // Set server address and port
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ip;
    serv_addr.sin_port = htons(port);

	printf("Connecting to p2p server ...\n");

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect error");
        return -1;
    }

	sleep(5);

    // Send file name to server
    if (send(sock, file_name, strlen(file_name), 0) < 0) {
        perror("send file name error");
        return -1;
    }


    // Receive file contents from server
    fp = fopen(file_name, "wb");
    if (fp == NULL) {
        perror("fopen file error");
        return -1;
    }


	/***********************************************
	 * Refer to the description of file transfer
	 * process to receive and save the file.
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	// This part also require double checking, easy to implement wrong
	
	unsigned long file_len ;

	// receive the length of the file
	// if there are any errors occur
	// call perror and exit(EXIT_FAILURE)
	if (recv(sock, &file_len, sizeof(file_len), 0) < 0)
	{
		perror("recv file length error") ;
		exit(EXIT_FAILURE) ;
	}

	// initialize number of bytes received to be 0
	unsigned long no_bytes_received = 0 ;

	// use while loop to get all the content inside the file
	while (no_bytes_received < file_len)
	{
		bytes_read = recv(sock, buffer, MAXMSG, 0) ;

		// if the received byted has error
		// use perror and exit(EXIT_FAILURE)
		if (bytes_read < 0)
		{
			perror("recv file content error") ;
			exit(EXIT_FAILURE) ;
		}

		// size_t fwrite( const void* restrict buffer, 
		//				  size_t size, 
		// 				  size_t count, 
		//				  FILE* restrict stream );
		//
		// buffer: pointer to the first object in the array to be written
		// size: size of each object
		// count: the number of the objects to be written
		// stream: pointer to the output stream
		//
		// Return value:
		// The number of objects written successfully, which may be less than count if an error occurs.
		// If size or count is zero, fwrite returns zero and performs no other action.

		// write the content to the corresponding file
		fwrite(buffer, sizeof(char), bytes_read, fp) ;

		// increment number of bytes received by 
		// the actual number of bytes received
		no_bytes_received += bytes_read ;

	}

	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/

	printf("Receive finished !\n");

    fclose(fp);
    close(sock);
    return 0;
}


void clear (void)
{
    while ( getchar() != '\n' );
}


int main() {


	/***** BONUS part START *****/

	// initialize an array of struct ip_port to store the query records
	// initialize a variable to store the number of query records

	struct ip_port query_record[MAX_NO_QUERY] ;
	int no_query = 0 ;

	/***** BONUS part END *****/


	char ip_string[20] = "127.0.0.1";
	unsigned short port = 6000;

	printf("\nInput ip address: ");
	scanf("%s", ip_string);
	unsigned int ip = inet_addr(ip_string);

	printf("\nInput port number: ");
	scanf("%hu", &port);

	/* start p2p server service */
	pthread_t tid;
	struct ip_port arg;
	arg.ip = ip;
	arg.port = port;

	printf("Creating p2p server ...\n");

	/***********************************************
	 * Start the p2p server using pthread
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	// pthread_t* thread: Allocate thread before calling.
	// const pthread_attr_t * attr: We won’t use and just set NULL.
	// void* (*start_routine ) ( void *): The function that the thread is to run.
	// void* arg_p: Pointer to the argument that should be passed to the function start_routine

	// Return value: on success, pthread_create() returns 0; on error, it returns an error

	// pthread_t* thread should be &tid
	// const pthread_attr_t * attr should be set to NULL as we would not use it
	// void* (*start_routine ) ( void *) should be set to p2p_server function
	// void* arg_p should be set to (void *) arg

	// if pthread_create hav error, it returns error number 
	// and we need to use perror and exit(EXIT_FAILURE) to indicate that
	if (pthread_create(&tid, NULL, &p2p_server, (void *) &arg) != 0)
	{
		perror("pthread creation error") ;
		exit(EXIT_FAILURE) ;
	}

	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/

	sleep(1);


	int sockfd;
	struct sockaddr_in servaddr;

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERVER);
	servaddr.sin_port = htons(SERVER_PORT);

	char command[100];
	char file_name[256];
	unsigned int local_map;

	char register_flag = 0;
	char query_flag = 0;

	do {
		clear();

		bzero(command, sizeof(command));

		printf("\nInput command: ");
		scanf("%s", command);

		if (strncmp(command, REGISTER, strlen(REGISTER)) == 0) {
			send_register(sockfd, servaddr, ip, port);
			register_flag = 1;
			continue;
		}

		if (strncmp(command, UPDATE, strlen(UPDATE)) == 0) {
			if ( register_flag == 0 ) {
				printf("You should first register!");
			} else {
				local_map = get_file_map();
				send_update(sockfd, servaddr, ip, port, local_map);
			}
			continue;
		}

		if (strncmp(command, QUERY, strlen(QUERY)) == 0) {
			printf("\nInput file name: ");
			scanf("%s", file_name);
			// send_query(sockfd, servaddr, file_name, strlen(file_name));


			/***** BONUS part START *****/

			// get and store the number of query recorda available
			no_query = send_query(sockfd, servaddr, file_name, strlen(file_name), query_record) ;
			
			/***** BONUS part END *****/


			query_flag = 1;
			continue;
		}

		if (strncmp(command, GET, strlen(GET)) == 0) {

			if ( query_flag == 0 ) {
				printf("You should first query some files!");
			} else {
				char input_ip[32];
				unsigned short input_port;

				/***** BONUS part START *****/

				// use a char array to store the terminal input from user
				char terminal_input[100] ;

				// count how many arguments in the terminal input by the user
				// int terminal_input_argument_no = 0 ;

				// prompt the user have alternative choice to input the sequence number
				printf("\nInput ip port (e.g., 127.0.0.1 6001): ");
				printf("\nBONUS part: otherwise, please input the sequence number (must be >= 0), such as 3: ") ;

				// discard the '\n' follows by the last character input by user in terminal
				clear() ;

				// char *fgets( char *restrict str, int count, FILE *restrict stream );
				//
				// str: pointer to an element of a char array
				// count: maximum number of characters to write (typically the length of str)
				// stream: file stream to read the data from
				//
				// Return value:
				// str on success, null pointer on failure.

				// read the first 100 characters (including '\0') input by user from the terminal
				// and store it inside terminal_input char array
				fgets(terminal_input, 100, stdin) ;

				// int ungetc( int ch, FILE *stream );
				//
				// ch: character to be pushed into the input stream buffer
				// stream: file stream to put the character back to
				//
				// Return value:
				// On success ch is returned.
				// On failure EOF is returned and the given stream remains unchanged.

				// push back '\n' to the stdin stream for next read operation
				ungetc('\n', stdin) ;

				// int sscanf ( const char * s, const char * format, ...);
				// 
				// s: C string that the function processes as its source to retrieve the data.
				// format: C string that contains a format string that follows the same specifications as format in scanf (see scanf for details).
				// ...: additional arguments, which is optional
				//
				// Return value:
				// On success, the function returns the number of items in the argument list successfully filled. 
				// This count can match the expected number of items or be less (even zero) in the case of a matching failure.
				// In the case of an input failure before any data could be successfully interpreted, EOF is returned.

				// Case A: the standard way, 2 arguments (ip & port)
				if (strlen(terminal_input) >= 8)
				{
					// get and store the IP and port number
					sscanf(terminal_input, "%s %hu", input_ip, &input_port) ;

					// call p2p_client() as normal
					p2p_client(inet_addr(input_ip), input_port, file_name) ;
				}

				// Case B: the BONUS way, 1 argument (sequence number)
				else
				{
					// initialize a variable to store the target sequence number
					int target_sequence_no = -1 ;

					// get and store the target sequence number
					sscanf(terminal_input, "%d", &target_sequence_no) ;

					// report error if the target sequence number is out of range
					// and skip to next iteration
					if (target_sequence_no < 0 || target_sequence_no >= no_query)
					{
						printf("\nThe corresponding sequence number not exist!\n") ;

						continue ;
					}

					// otherwise, if the target sequence number is in the range
					// call p2p_client()
					p2p_client(query_record[target_sequence_no].ip, query_record[target_sequence_no].port, file_name) ;

				}


				/***** BONUS part END *****/

				// printf("\nInput ip port (e.g., 127.0.0.1 6001): ");
				// scanf("%s %hu", input_ip, &input_port);

				// p2p_client(inet_addr(input_ip), input_port, file_name);

				sleep(2);

				/* then update */
				local_map = get_file_map();
				send_update(sockfd, servaddr, ip, port, local_map);
			}

			continue;
		}

		if (strncmp(command, EXIT, strlen(EXIT)) == 0) {
			printf("Exit!");
			return 0;
		}

		printf("\nValid commands: REGISTER | UPDATE | QUERY | GET | EXIT\n");

	} while (1);

    // Wait for server thread to finish
    if (pthread_join(tid, NULL) != 0) {
        perror("pthread_join");
        return 1;
    }

	close(sockfd);
	return 0;
}
