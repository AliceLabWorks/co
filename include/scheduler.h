#pragma once

#include "corotine.h"
#include <list>
#include <mutex>
#include <unordered_set>

namespace ab {
class ITaskScheduler {
public:
  virtual Corotine *ScheduleOne() = 0;
  virtual void TaskReady(Corotine *) = 0;
  virtual void TaskWait(Corotine *) = 0;
  virtual void TaskYield(Corotine *) = 0;
};

class ColangScheduler : public ITaskScheduler {
public:
  ColangScheduler() {}
  virtual Corotine *ScheduleOne() override final;
  virtual void TaskReady(Corotine *co) override final;
  virtual void TaskWait(Corotine *co) override final;
  virtual void TaskYield(Corotine *co) override final;

  std::mutex mutex_;
  std::list<Corotine *> ready_list_;
  std::unordered_set<Corotine *> wait_list_;
};
} // namespace ab