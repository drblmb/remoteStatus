#include <iostream>
#include <strings.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "TcpClient.h"

using namespace std;

TcpClient *
TcpClient::create()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return 0;
  return new TcpClient(sock);
}

TcpClient::TcpClient(int fd)
  : TcpSocket(fd)
{
}

TcpClient::~TcpClient()
{
}

bool
TcpClient::connect(char *host, unsigned short port)
{
  struct hostent *server;

  server = gethostbyname(host);
	if (server == NULL) 
	{
    perror("Error:");
    return false;
	}

  //  Try to connect to host
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length); 
  addr.sin_port = htons(port);
  while (::connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    if (errno == EINTR)
      continue;
    else {
      perror("Error: ");
      return false;
    }
  }
  return true;
}

#if 0
bool
TcpClient::connect(const char* host, unsigned short port, unsigned long timeout)
{
  //  Lookup IP address for host
  struct hostent *h = gethostbyname(host);
  if (!h)
    return false;

  //  Try to connect to host
  return connect(((struct in_addr *) (h->h_addr_list[0]))->s_addr, port, timeout);
}
#endif
