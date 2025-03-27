#include "corotine.h"

namespace ab {
void Corotine::Resume() {
  auto &thread_info = ThisCoro::GetThreadInfo();
  assert(thread_info.running_coro == nullptr);
  thread_info.running_coro = this;
  thread_info.async_safe_point = false;
  fiber_ = std::move(fiber_).resume();
  assert(thread_info.async_safe_point == false);
  thread_info.running_coro = nullptr;
  if (thread_info.handling_signal.load()) {
    thread_info.handling_signal = false;
  }
}

void Corotine::Yield() {
  auto &thread_info = ThisCoro::GetThreadInfo();
  assert(thread_info.async_safe_point == true);
  thread_info.async_safe_point = false;
  main_ = std::move(main_).resume();
  assert(thread_info.async_safe_point == false);
  thread_info.async_safe_point = true;
}

bool Corotine::Finished() const { return static_cast<bool>(fiber_) == false; }
CoRet Corotine::GetRet() const { return ret_; }
uint64_t Corotine::GetID() const { return id_; }
void Corotine::SetRetVal(int val) { ret_.ret_code = val; }
void Corotine::SetRetMsg(const std::string &msg) { ret_.ret_msg = msg; }
} // namespace ab