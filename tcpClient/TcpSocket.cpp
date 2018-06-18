#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include "TcpSocket.h"


TcpSocket::TcpSocket(int fd)
  : sock(fd),
    auto_close(true)
{
}

TcpSocket::~TcpSocket()
{
  if (auto_close) {
    ::close(sock);
  }
}

void
TcpSocket::disable_auto_close()
{
  auto_close = false;
}

void
TcpSocket::set_send_timeout(int msec)
{
  struct timeval t;
  t.tv_sec  =  msec / 1000;
  t.tv_usec = (msec % 1000) * 1000;
  // @JP@ repetitive code shall be replaced by helper function
  // @JP@ in modern C++, std::chrono types are more appropriate

  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(struct timeval)) < 0) {
    //LOG(LOGID_ERROR, "Failed to set socket send timeout: %m");
  }
}

void
TcpSocket::set_recv_timeout(int msec)
{
  struct timeval t;
  t.tv_sec  =  msec / 1000;
  t.tv_usec = (msec % 1000) * 1000;

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(struct timeval)) < 0) {
    //LOG(LOGID_ERROR, "Failed to set socket recv timeout: %m");
  }
}

bool
TcpSocket::send(const void* buf, size_t len)
{
  char* p = (char*) buf;
  while (len > 0) {
    ssize_t nsent = ::send(sock, p, len, 0);

    if (nsent <= 0) {
      if (nsent < 0) {
        if (errno == EINTR)  continue;
        //if (errno == EAGAIN) LOG(LOGID_ERROR, "timeout");
        //else                 LOG(LOGID_ERROR, "error: %m");
      }
      //LOG(LOGID_ERROR, "unknown error");
      return false;
    }

    len -= nsent;
    p += nsent;
  }
  return true;
}

bool
TcpSocket::write(const char* str)
{
  return send(str, strlen(str));
}

bool
TcpSocket::send(const char* str)
{
  return send(str, strlen(str) + 1);
}

bool
TcpSocket::read(void* buf, size_t* len, bool* timeout, int msecs)
{
  //  Prepare sets for SELECT
  fd_set fds;

  //  Prepare timeout for SELECT
  struct timeval t;
  t.tv_sec = msecs / 1000;
  t.tv_usec = (msecs % 1000) * 1000;

  //  Loop until we read the requested amount of data
  char* p = (char*) buf;
  size_t sz = *len;
  *len = 0;
  while (sz > 0) {
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    int n = select(sock + 1, &fds, 0, 0, (timeout == 0) ? 0 : &t);
    if (n < 0)
      return false;
    if (timeout)
      *timeout = (n == 0);
    if (n == 0)
      return true;
    if (!FD_ISSET(sock, &fds)) {
      break;
    }
    ssize_t nread = recv(sock, p, sz, 0);
    if (nread < 0)
      return false;
    if (nread == 0)
      return true;
    sz -= nread;
    p += nread;
    *len += nread;
  }
  return true;
}


int
TcpSocket::readSome(void* buf, size_t len, bool* timeout, int msecs)
{
  //  Prepare sets for SELECT
  fd_set fds;

  //  Prepare timeout for SELECT
  struct timeval t;
  t.tv_sec = msecs / 1000;
  t.tv_usec = (msecs % 1000) * 1000;

  // delay until we get some data or timeout
  // @JP@ DRY principle not met
  FD_ZERO(&fds);
  FD_SET(sock, &fds);
  int n = select(sock + 1, &fds, 0, 0, (timeout == 0) ? 0 : &t);
  if (n < 0)
    return -1;
  if (timeout)
    *timeout = (n == 0);
  if (n == 0)
    return 0;
  if (!FD_ISSET(sock, &fds))
    return -1;

  ssize_t nread = recv(sock, buf, len, 0);
  if (nread < 0)
    return -1;

  return nread;
}

// simpler alternative to 'read' above which doesnt poll/select and treats timeout as err
bool
TcpSocket::read_buf(void *buf, size_t len)
{
  char* p = (char*) buf; // @JP@ c-style cast
  while (len > 0) {
    ssize_t nread = recv(sock, p, len, 0);

    if (nread <= 0) {
      if (nread < 0) {
        if (errno == EINTR)  continue;
        //if (errno == EAGAIN) //LOG(LOGID_ERROR, "timeout");
        //else                 //LOG(LOGID_ERROR, "error: %m");
      }

      // peer has performed an orderly shutdown.
      return false;
    }

    len -= nread;
    p += nread;
  }
  return true;
}
