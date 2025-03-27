#include "scheduler.h"

namespace ab {
Corotine *ColangScheduler::ScheduleOne() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (ready_list_.empty())
    return nullptr;
  Corotine *coro = ready_list_.front();
  ready_list_.pop_front();
  return coro;
}

void ColangScheduler::TaskReady(Corotine *co) {
  std::lock_guard<std::mutex> lock(mutex_);
  ready_list_.push_back(co);
  auto iter = wait_list_.find(co);
  if (iter != wait_list_.end()) {
    wait_list_.erase(iter);
  }
}

void ColangScheduler::TaskWait(Corotine *co) {
  std::lock_guard<std::mutex> lock(mutex_);
  wait_list_.insert(co);
}

void ColangScheduler::TaskYield(Corotine *co) {
  std::lock_guard<std::mutex> lock(mutex_);
  ready_list_.push_back(co);
}
}; // namespace ab