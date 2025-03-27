#pragma once

#include "co_sock.h"
#include "singleton.h"
#include <functional>
#include <map>
#include <sys/epoll.h>
#include <unistd.h>

namespace ab {

struct TimerIndex {
  uint32_t awake_time;
  uint32_t timer_id;
  bool is_repeat;
  bool operator<(const TimerIndex &other) const {
    return std::tie(awake_time, timer_id) <
           std::tie(other.awake_time, other.timer_id);
  }
};

class Poller : public Singleton<Poller> {
public:
  Poller() {}
  bool Init(pthread_t main_thd_id, uint32_t timeout_ms = 200);
  ~Poller();
  int AddSocket(CoSocket &co_sock, uint32_t events);
  void ModifySocket(int fd, uint32_t events);
  int RemoveSocket(CoSocket &co_sock);
  void Run();
  void AddTimer(uint32_t interval_ms, bool is_repeat, Corotine *co);
  void NtfSignal(bool force = false);

private:
  void HandleRead(CoSocket *co_sock);
  void HandleWrite(CoSocket *co_sock);
  void HandleTimers();

private:
  int epoll_fd_ = -1;

  std::map<TimerIndex, Corotine *> timers_;
  uint32_t timeout_ms_;
  pthread_t main_thd_id_ = 0;
};
} // namespace ab