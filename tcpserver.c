// Our standard libraries.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Some types and unix operations.
#include <sys/types.h>
#include <unistd.h>

// A sockets library and internet protocol library.
#include <sys/socket.h>
#include <netinet/in.h>

#define ARBITRARY_PORT 15200 // Change this if bind failure.
#define MAX_CLIENTS 5 // Max number of clients in wait queue.
#define MESSAGE_SIZE 128 // Max number of characters (i.e. bytes).


// Helper method to read from a client socket to a buffer.
// Returns number of bytes read.
// Will close the server's socket and exit if error occurs.
int read_from_client(int client_socket, int server_socket, char* buffer){
	int r = recv(client_socket, buffer, MESSAGE_SIZE * sizeof(char), 0);
	if (r < 0){
		fprintf(stderr, "Error receiving client message.\n");
		close(server_socket);
		exit(1);
	}
	return r;
}

// Helper method to parse input string for use with execvp.
// Return value is dynamically allocated and must be freed.
char** parse_input(char* input) {
	char** exec_args = malloc(MESSAGE_SIZE * sizeof(char*));

	int count = 0;
	char delimiter[]= " \t\r\n\v\f";
	char* token = strtok(input, delimiter);
	while (token != NULL && count < MESSAGE_SIZE) {
		exec_args[count] = token;
		count++;
		token = strtok(NULL, delimiter);
	}
	exec_args[count] = NULL;
	return exec_args;
}

// Helper method to execute a parsed client command. Will close the
// forked process and its socket if command is not valid.
void execute_command(char* input, int socket){
	char** exec_args = parse_input(input);

	pid_t pid;
	pid = fork();
	if(pid == 0) {
		execvp(exec_args[0], exec_args);
		printf("Command not recognized.\n");
		close(socket);
		free(exec_args);
		exit(1);
	}
	waitpid(pid, NULL, 0);
	free(exec_args);
}

// Program for the server in a client-server network setup.
int main() {

    	// Create the server socket. IPv4 TCP.
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

    	// Define the socket address format - assigned to localhost.
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(ARBITRARY_PORT);
	server_address.sin_addr.s_addr = htons(INADDR_ANY);

	// Bind address to socket.
    	if (bind(server_socket, (struct sockaddr*) &server_address,
			sizeof(server_address)) < 0){
		fprintf(stderr, "Could not bind socket to address.\n");
		close(server_socket);
		exit(1);	
	}
	printf("Socket bound to localhost at port %d.\n", ARBITRARY_PORT);

	// Begin listening for client connection attempts.
    	listen(server_socket, MAX_CLIENTS);
	printf("Server is now listening for a client.\n");

    	// Wait for client connection requests until the end of time.
	int client_socket;
	while(1){
	if ((client_socket = accept(server_socket, NULL, NULL)) < 0){
		fprintf(stderr, "Error accepting incoming client.\n");
		close(server_socket);
		exit(1);		
	}
	printf("A client has connected via socket %d.\n", client_socket);
	
	// Get client's username then greet them.
	char input[MESSAGE_SIZE];
	read_from_client(client_socket, server_socket, input);
	char server_message[29] = "You have reached the server.";
	send(client_socket, server_message, 29 * sizeof(char), 0);

	// Accept this client's commands until  client disconnect (bytes
	// no longer read). Then listen for other client connect requests.
	printf("> %s\n", input);
	int r = read_from_client(client_socket, server_socket, input);
	while(r) {
		printf("%s\n", input);
		execute_command(input, client_socket);
		r = read_from_client(client_socket, server_socket, input);
	}
	
	printf("\nServer is now listening for a client.\n");
	}

	// Close the sockets.
	// In the current implementation, this is an unreachable
	// statement.  Left in as a reminder in case things change.
	close(server_socket);
	close(client_socket);
	return 0;
}

