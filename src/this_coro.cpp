#include "colang_server.h"
#include "corotine.h"
#include "this_coro.h"

namespace ab {
thread_local static ColangSvrThreadInfo thread_info;

AsyncSafeGuard::AsyncSafeGuard()
    : async_safe_point_(thread_info.async_safe_point) {
  thread_info.async_safe_point = false;
}
AsyncSafeGuard::~AsyncSafeGuard() {
  thread_info.async_safe_point = async_safe_point_;
}

void ThisCoro::Yield() {
  Corotine *co = thread_info.running_coro;
  ColangServer::GetInst().GetTaskScheduler()->TaskYield(
      thread_info.running_coro);
  co->Yield();
}

void ThisCoro::Sleep(uint32_t interval_ms) {
  Corotine *co = thread_info.running_coro;
  ColangServer::GetInst().GetTaskScheduler()->TaskWait(co);
  Poller::GetInst().AddTimer(interval_ms, false, co);
  co->Yield();
}

void ThisCoro::SetRet(int val) {
  Corotine *co = thread_info.running_coro;
  co->SetRetVal(val);
}

void ThisCoro::SetRet(int val, const std::string &msg) {
  Corotine *co = thread_info.running_coro;
  co->SetRetVal(val);
  co->SetRetMsg(msg);
}

bool ThisCoro::IsAsyncSafePoint() {
  return thread_info.async_safe_point == true &&
         thread_info.running_coro != nullptr &&
         thread_info.handling_signal.load() == false;
}

ColangSvrThreadInfo &ThisCoro::GetThreadInfo() { return thread_info; }
} // namespace ab

extern "C" __attribute__((noinline)) void YieldBySignal() {
  ab::Corotine *co = ab::thread_info.running_coro;
  ab::ColangServer::GetInst().GetTaskScheduler()->TaskYield(co);
  co->Yield();
}