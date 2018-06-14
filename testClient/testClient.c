#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFF_SIZE 256

int main(int argc, char **argv)
{
	int socketFd, n;
	struct sockaddr_in serverAddr;
	struct hostent *server;
	unsigned short targetPort;
	char buf[BUFF_SIZE];

	if (argc != 3) 
	{
		printf("Usage: ./testClient <server> <port>\n");
		exit(-1);
	}

	server = gethostbyname(argv[1]);
	if (server == NULL) 
	{
		perror("Error, host not found");
		exit(-1);
	}
	targetPort = (unsigned short)strtol(argv[2], 0, 10);

	/* socket: create the socket */
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd < 0) {
		perror("ERROR opening socket");
		exit(-1);
	}

	bzero((char *)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
	serverAddr.sin_port = htons(targetPort);

	/* connect: create a connection with the server */
	if (connect(socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("ERROR connecting to server");
		exit(-1);
	}

	/* there should be a way to exit besides ctrl-c, but choosing the simple approach */
	while(1)
	{
		printf("Enter command ('mem' or 'cpu') ->");
		fgets(buf, BUFF_SIZE, stdin);
		n = write(socketFd, buf, strlen(buf)+1);
		if (n < 0) 
		{
			perror("Error writing to socket");
			break;
		}
		/* get result - brittle because it relies on the server always providing a response.
		   A better approach is to use non-blocking io or select/events */
		/* read answer */
		n = read(socketFd, buf, BUFF_SIZE);
		if (n < 0) {
			perror("Error reading from socket");
			break;
		}
		printf("%s\n\n", buf);
	}

	close(socketFd);
	return 0;
}

