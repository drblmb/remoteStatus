#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include <netinet/in.h>
#include "TcpSocket.h"


class TcpClient : public TcpSocket {
public:
  static TcpClient* create();
  virtual ~TcpClient();

  bool connect(char *host, unsigned short port);

private:
   TcpClient(int fd);
};

#endif // _TCP_CLIENT_H_
