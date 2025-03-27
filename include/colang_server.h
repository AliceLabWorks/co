#pragma once

#include "coro_alloc.h"
#include "corotine.h"
#include "poller.h"
#include "scheduler.h"
#include "singleton.h"
#include "this_coro.h"
#include <functional>
#include <thread>

namespace ab {
class ColangServer : public Singleton<ColangServer> {
public:
  ColangServer();
  ITaskScheduler *GetTaskScheduler() { return &scheduler_; }

  void CoPoll();
  int Start(CoFunc func);

  template <typename CoFunc> Corotine *Spawn(CoFunc &&func, bool start = true);

  template <typename CoFunc>
  int AddTimer(CoFunc &&timer_func, uint32_t interval_ms,
               bool is_repeat = false);
  Corotine *ThisCoro();

private:
  std::thread poller_thd_;
  ColangScheduler scheduler_;
};

template <typename CoFunc>
Corotine *ColangServer::Spawn(CoFunc &&func, bool start) {
  auto new_co = [func = std::forward<CoFunc>(func), start]() mutable {
    Corotine *co = CoroAllocator::GetInst().Alloc();
    co->Init(std::forward<CoFunc>(func));
    if (start) {
      ColangServer::GetInst().GetTaskScheduler()->TaskReady(co);
    }
    return co;
  };
  if (ThisCoro::GetThreadInfo().running_coro == nullptr) {
    return new_co();
  } else {
    AsyncSafeGuard guard;
    return new_co();
  }
}

template <typename Func>
int ColangServer::AddTimer(Func &&timer_func, uint32_t interval_ms,
                           bool is_repeat) {
  static_assert(std::is_constructible_v<CoFunc, Func &&>,
                "Func must be constructible to CoFunc");
  if (timer_func == nullptr && is_repeat == true) {
    return -1;
  }
  Corotine *coro =
      ColangServer::GetInst().Spawn(std::forward<Func>(timer_func), false);
  Poller::GetInst().AddTimer(interval_ms, is_repeat, coro);
  return 0;
}

template <typename Func> decltype(auto) co(Func &&func) {
  return [func = std::forward<Func>(func)](auto &&...args) mutable {
    ColangServer::GetInst().Spawn(
        [func = std::move(func),
         args = std::forward_as_tuple(
             std::forward<decltype(args)>(args)...)]() mutable {
          std::apply(std::move(func), std::move(args));
        });
  };
}
} // namespace ab
