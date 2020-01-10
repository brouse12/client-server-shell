#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <netinet/in.h>

#define ARBITRARY_PORT 15200 // Must match server's port.
#define MESSAGE_SIZE 128 // Max number of characters (i.e. bytes)

// Helper method to get user input and send it to the server.
// Will close the socket and exit if error occurs or if user sends
// an exit command..
void send_input_to_server(int socket){
	char input[MESSAGE_SIZE];
	fgets(input, MESSAGE_SIZE, stdin);
	
	// Exit if user typed exit.
	if(strcmp(input, "exit\n") == 0){
		close(socket);
		exit(0);
	}

	// Else send the message to specified socket.
	int message_length = strlen(input);
	input[message_length - 1] = '\0'; // Strip '\n'
	int s = send(socket, input, message_length * sizeof(char), 0);
	if (s < 0){
		fprintf(stderr, "Error sending message.\n");
		close(socket);
		exit(1);
	}
}

// Program for the client in a client-server network setup.
int main(){

   	// Create a socket for a IPv4 TCP connection.
	int client_socket;
	if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Socket creation error\n");
	}

	// Set the socket address to localhost and an arbitrary port.
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; 
	server_address.sin_port = htons(ARBITRARY_PORT); 
	server_address.sin_addr.s_addr = htons(INADDR_ANY); 
    
    	// Attempt to connect.
	printf("Attempting to connect to localhost at port %d.\n",
		ARBITRARY_PORT);
	int connection_status;
	if((connection_status = connect(
		client_socket, (struct sockaddr*)&server_address,
			sizeof(server_address))) < 0){
		fprintf(stderr, "%d Connection unsuccessful.\n",
				connection_status);
		close(client_socket);
		exit(0);
	}	

	// Get username to be used on the server.
	printf("Connected successfully.\n"
		"What is your username?\n");
	send_input_to_server(client_socket);

   	// Wait for opening on server then print the server's greeting.
	printf("Waiting in server queue . . .\n");
	char server_response[MESSAGE_SIZE];
	recv(client_socket, &server_response, 
		MESSAGE_SIZE * sizeof(char), 0);
	printf("The server sent the data: %s\n", server_response);

	// Enter executable server commands until exit is called.
	while(1) {
		printf("client>");
		send_input_to_server(client_socket);
	}

	// Then close the socket
	close(client_socket);

    return 0;
}

