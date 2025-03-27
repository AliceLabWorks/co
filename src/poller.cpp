#include "co_sock.h"
#include "colang_server.h"
#include "poller.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <map>
#include <signal.h>
#include <sys/epoll.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace ab {

bool Poller::Init(pthread_t main_thd_id, uint32_t timeout_ms) {
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    perror("Poller::Init epoll_create1");
    return false;
  }
  timeout_ms_ = timeout_ms;
  main_thd_id_ = main_thd_id;
  return true;
}

Poller::~Poller() { close(epoll_fd_); }

int Poller::AddSocket(CoSocket &co_sock, uint32_t events) {
  struct epoll_event event;
  event.data.ptr = &co_sock;
  event.events = events;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, co_sock.Getfd(), &event) == -1) {
    return -1;
  }
  return 0;
}

void Poller::ModifySocket(int fd, uint32_t events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) == -1) {
    perror("epoll_ctl: modify_socket");
    exit(EXIT_FAILURE);
  }
}

int Poller::RemoveSocket(CoSocket &co_sock) {
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, co_sock.Getfd(), nullptr) == -1) {
    return -1;
  }
  return 0;
}

void Poller::NtfSignal(bool force) {
  int ret = pthread_kill(main_thd_id_, SIGUSR1);
  if (ret != 0) {
    std::cerr << "Failed to send signal to thread: " << strerror(ret)
              << std::endl;
  }
}

void Poller::Run() {
  while (true) {
    static constexpr int MAX_EVENTS = 1024;
    static struct epoll_event events[MAX_EVENTS];
    int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, timeout_ms_);
    if (n == -1) {
      continue;
    }

    for (int i = 0; i < n; ++i) {
      CoSocket *co_sock = (CoSocket *)(events[i].data.ptr);
      uint32_t ev = events[i].events;
      if (ev & EPOLLIN) {
        HandleRead(co_sock);
      }
      if (ev & EPOLLOUT) {
        HandleWrite(co_sock);
      }
    }
    HandleTimers();
    NtfSignal();
  }
}

void Poller::AddTimer(uint32_t interval_ms, bool is_repeat, Corotine *co) {
  static uint32_t timer_id = 0;

  auto now = std::chrono::steady_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch())
                    .count();

  // 计算唤醒时间
  uint32_t awake_time = static_cast<uint32_t>(now_ms + interval_ms);
  // 将定时器添加到 timers_ 中
  timers_.emplace(TimerIndex{awake_time, timer_id++, is_repeat}, co);
}

void Poller::HandleRead(CoSocket *co_sock) {
  RemoveSocket(*co_sock);
  ColangServer::GetInst().GetTaskScheduler()->TaskReady(co_sock->OnCoro());
}

void Poller::HandleWrite(CoSocket *co_sock) {
  RemoveSocket(*co_sock);
  ColangServer::GetInst().GetTaskScheduler()->TaskReady(co_sock->OnCoro());
}

void Poller::HandleTimers() {
  auto now = std::chrono::steady_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch())
                    .count();
  for (auto iter = timers_.begin(); iter != timers_.end(); ++iter) {
    const TimerIndex &timer_index = iter->first;
    Corotine *coro = iter->second;
    if (now_ms >= timer_index.awake_time) {
      ColangServer::GetInst().GetTaskScheduler()->TaskReady(coro);
      iter = timers_.erase(iter);
    } else {
      break;
    }
  }
}
} // namespace ab