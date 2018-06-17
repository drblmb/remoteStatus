#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include <stddef.h>
#include <netinet/in.h>

class TcpSocket {
public:
  TcpSocket(int fd);
   virtual ~TcpSocket();

  bool send(const void *buf, size_t len);
  bool send(const char *str);

  bool read(void *buf, size_t * len, bool * timeout = 0, int msecs = 0);
  int readSome(void *buf, size_t len, bool * timeout = 0, int msecs = 0);

  // simpler alternative to 'read' above which doesnt poll/select and treats timeout as err
  bool read_buf(void *buf, size_t len);   // return: false (incomplete read)

  bool write(const char *str);

  int fd();
  void disable_auto_close();

  void set_send_timeout(int msecs);
  void set_recv_timeout(int msecs);

  // enable:   true: enable keep alives and if any following values > 0 override kernel defaults
  // idle:     the interval (s) between the last data packet sent and the first keepalive probe
  // interval: interval (s) between keep alive probes
  // count:    number of unacknowledged probes to send before considering the connection dead
  void set_keepalive(bool enable, int idle = 0, int interval = 0, int count = 0);

  void set_nodelay(bool enable);

protected:
  int sock;
  bool auto_close;
};

inline int TcpSocket::fd() { return sock; }

#endif // _TCP_SOCKET_H_
