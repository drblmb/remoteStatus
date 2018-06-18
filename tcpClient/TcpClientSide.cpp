#include <iostream>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "TcpClientSide.h"
#include "TcpTypes.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TcpClientSide::TcpClientSide(char * server, UINT16 port)
{
	strcpy(target_host, server);
	target_port = port;
	keep_running = true;
}

TcpClientSide::~TcpClientSide()
{
}

bool
TcpClientSide::SendAndReceive()
{
	char buf[BUFFLEN];
	//  Create socket
	client = TcpClient::create();
	if (!client || !client->connect(target_host, target_port)) {
		perror("Error: ");
		return false; // @JP@ client memory leak here, see my other comment related to a smart ptr
	}

	while(keep_running) {
		cout << "Please enter command - ('mem', 'cpu', 'exit') ->";
		cin >> buf;
		if (!strncmp(buf, "exit", 4)) {
			keep_running = false;
			continue;
		}

		// Make sure command is terminated by newline
		if (buf[strlen(buf)-1] != '\n') {
			strcat(buf, "\n");
		}

		client->send(buf);

		if (client->readSome(buf, BUFFLEN, NULL, 0) < 0) {
			perror("Error reading: ");
			keep_running = false;
			continue;
		}

		cout << buf << "\n";
	}
	//  Close socket
	delete client;
	return true;
}

int main(int argc, char **argv) {
	if (argc !=  3) {
	  cout << "Usage: tcpClient <server> <port>\n";
	  return -1;
	}

	TcpClientSide *tcpClient = new TcpClientSide(argv[1], atoi(argv[2]));
	// @JP@ tcpClient memory leak here; I'm aware of exiting the program here, but why don't allocate the tcpClient on Stack?
	// TcpClientSide tcpClient(argv[1], atoi(argv[2]));
	// tcpClient.SendAndReceive();

	tcpClient->SendAndReceive();	

	// @JP@ the code inside main() function can throw an exception; there is no try-catch block handling this properly
  }

