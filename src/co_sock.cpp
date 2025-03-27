#include "co_sock.h"
#include "colang_server.h"
#include "poller.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

namespace ab {

bool CoSocket::InitProtocol(bool ipv4, bool tcp) {
  int domain = ipv4 ? AF_INET : AF_INET6;
  int type = tcp ? SOCK_STREAM : SOCK_DGRAM;
  fd_ = ::socket(domain, type, 0);
  if (fd_ == -1) {
    return false;
  }
  return SetNonBlocking();
}

CoSocket::~CoSocket() {
  if (fd_ != -1) {
    close(fd_);
  }
}

int CoSocket::Getfd() const { return fd_; }

int CoSocket::Bind(const std::string &host, int port) {
  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  int ret = inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);
  if (ret != 1)
    return ret;
  return ::bind(fd_, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

int CoSocket::Listen(int backlog) { return ::listen(fd_, backlog); }

int CoSocket::Connect(const std::string &host, int port) {
  assert(ColangServer::GetInst().ThisCoro());
  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  ::inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);
  while (true) {
    int ret =
        ::connect(fd_, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret) {
      if (errno != EINPROGRESS) {
        perror("connect");
        return ret;
      }
      ColangServer::GetInst().GetTaskScheduler()->TaskWait(
          ColangServer::GetInst().ThisCoro());
      ret = Poller::GetInst().AddSocket(*this, EPOLLOUT);
      if (ret) {
        return ret;
      }
      on_coro_ = ColangServer::GetInst().ThisCoro();
      on_coro_->Yield();
    } else {
      break;
    }
  }
  return 0;
}

int CoSocket::Accept(CoSocket &client_socket) {
  assert(ColangServer::GetInst().ThisCoro());
  sockaddr_in client_addr{};
  socklen_t addr_len = sizeof(client_addr);
  while (true) {
    int client_fd = ::accept(fd_, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        return errno;
      }
      ColangServer::GetInst().GetTaskScheduler()->TaskWait(
          ColangServer::GetInst().ThisCoro());
      int ret = Poller::GetInst().AddSocket(*this, EPOLLIN);
      if (ret) {
        return ret;
      }
      on_coro_ = ColangServer::GetInst().ThisCoro();
      on_coro_->Yield();
    } else {
      client_socket.fd_ = client_fd;
      return 0;
    }
  }
  return 0;
}

int CoSocket::Send(const std::string &data, size_t &len) {
  assert(ColangServer::GetInst().ThisCoro());
  size_t total_sent = 0;          // 记录已发送的字节数
  size_t data_size = data.size(); // 数据总大小

  while (total_sent < data_size) {
    ssize_t count =
        ::send(fd_, data.c_str() + total_sent, data_size - total_sent, 0);
    if (count < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        ColangServer::GetInst().GetTaskScheduler()->TaskWait(
            ColangServer::GetInst().ThisCoro());
        int ret = Poller::GetInst().AddSocket(*this, EPOLLOUT);
        if (ret) {
          return ret;
        }
        on_coro_ = ColangServer::GetInst().ThisCoro();
        on_coro_->Yield();
      } else {
        // 发生了其他错误，返回错误码
        return errno;
      }
    } else {
      // 更新已发送的字节数
      total_sent += count;
    }
  }
  len = total_sent;
  return 0;
}

int CoSocket::Recv(std::string &data, size_t &len) {
  assert(ColangServer::GetInst().ThisCoro());
  char buffer[1024];         // 创建一个缓冲区来接收数据
  size_t total_received = 0; // 记录已接收的字节数

  while (true) {
    ssize_t count = ::recv(fd_, buffer + total_received,
                           sizeof(buffer) - total_received, 0);
    if (count < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 如果没有数据可读，注册到 epoll 中并等待
        ColangServer::GetInst().GetTaskScheduler()->TaskWait(
            ColangServer::GetInst().ThisCoro());
        int ret = Poller::GetInst().AddSocket(*this, EPOLLIN);
        if (ret) {
          return ret;
        }
        on_coro_ = ColangServer::GetInst().ThisCoro();
        on_coro_->Yield();
      } else {
        // 发生了其他错误，返回错误码
        return errno;
      }
    } else if (count == 0) {
      // 连接已关闭
      return 0; // 或者返回一个特定的错误码，表示连接关闭
    } else {
      // 更新已接收的字节数
      total_received += count;
      data.assign(buffer, buffer + total_received);
      len = total_received;
      return 0;
    }
  }
}

bool CoSocket::SetNonBlocking() {
  int flags = fcntl(fd_, F_GETFL, 0);
  if (flags == -1)
    return false;
  return fcntl(fd_, F_SETFL, flags | O_NONBLOCK) != -1;
}
} // namespace ab