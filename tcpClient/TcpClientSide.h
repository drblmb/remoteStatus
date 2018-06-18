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
  char target_host[BUFFLEN]; // @JP@ std::string?
  UINT16 target_port; // @JP@ uint16_t?
  TcpClient* client; // @JP@ why hold as a member, the only usage is inside the SendAndReceive() method
  bool keep_running;
};

#endif // !defined(_TCP_CLIENT_SIDE_H)
