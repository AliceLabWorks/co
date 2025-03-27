#pragma once

#include "corotine.h"
#include <string>

namespace ab {
class CoSocket {
public:
  CoSocket() = default;
  CoSocket(const CoSocket &sock) = delete;
  CoSocket(CoSocket &&sock) : fd_(sock.fd_), on_coro_(sock.on_coro_) {
    sock.fd_ = -1;
    sock.on_coro_ = nullptr;
  };

  CoSocket &operator=(const CoSocket &sock) = delete;
  CoSocket &operator=(CoSocket &&sock) {
    fd_ = sock.fd_;
    on_coro_ = sock.on_coro_;
    sock.fd_ = -1;
    sock.on_coro_ = nullptr;
    return *this;
  }

  bool InitProtocol(bool ipv4, bool tcp);
  ~CoSocket();
  int Getfd() const;

  int Bind(const std::string &host, int port);
  int Listen(int backlog);
  int Connect(const std::string &host, int port);
  int Accept(CoSocket &client_socket);
  int Send(const std::string &data, size_t &len);
  int Recv(std::string &data, size_t &len);
  Corotine *OnCoro() { return on_coro_; }

private:
  bool SetNonBlocking();

private:
  int fd_ = -1;
  Corotine *on_coro_;
};
} // namespace ab