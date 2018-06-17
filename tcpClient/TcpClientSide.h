#if !defined(_TCP_CLIENT_SIDE_H)
#define _TCP_CLIENT_SIDE_H

#include "TcpClient.h"
#include "TcpTypes.h"

class TcpClientSide
{
public:
	TcpClientSide(char* server, UINT16 port);
	virtual ~TcpClientSide();

  bool SendAndReceive();

private:
  char target_host[BUFFLEN];
  UINT16 target_port;
  TcpClient* client;
  bool keep_running;
};

#endif // !defined(_TCP_CLIENT_SIDE_H)
