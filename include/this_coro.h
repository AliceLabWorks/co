#pragma once

#include <atomic>
#include <signal.h>
#include <string>

extern "C" void YieldBySignal();

namespace ab {
class Corotine;
struct ColangSvrThreadInfo {
  Corotine *running_coro = nullptr;
  bool async_safe_point = false;
  std::atomic_bool handling_signal = false;
};

struct ThisCoro {
  static void Yield();
  static void Sleep(uint32_t interval_ms);

  static void SetRet(int val);
  static void SetRet(int val, const std::string &msg);

  static bool IsAsyncSafePoint();

private:
  friend class ColangServer;
  friend class Corotine;
  friend void FailureSignalHandler(int signal_number, siginfo_t *signal_info,
                                   void *ucontext);
  static ColangSvrThreadInfo &GetThreadInfo();
};

class AsyncSafeGuard {
public:
  AsyncSafeGuard();
  ~AsyncSafeGuard();

private:
  bool async_safe_point_;
};

} // namespace ab
