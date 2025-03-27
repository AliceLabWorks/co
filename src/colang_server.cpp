#include "colang_server.h"
#include "coro_alloc.h"
#include "poller.h"
#include "this_coro.h"
#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace ab {

ColangServer::ColangServer() {
  if (Poller::GetInst().Init(pthread_self(), 2000) == false) {
    std::terminate();
  }
  poller_thd_ = std::thread(std::bind(&ColangServer::CoPoll, this));
  poller_thd_.detach();
}

Corotine *ColangServer::ThisCoro() {
  return ThisCoro::GetThreadInfo().running_coro;
}

void ColangServer::CoPoll() { Poller::GetInst().Run(); }

int ColangServer::Start(CoFunc func) {
  Corotine *co = Spawn(func);
  while (true) {
    co = scheduler_.ScheduleOne();
    // 如果执行的协程没结束,那就说明是某种原因切出来了
    // 那就由切出的方法来把协程重新放回调度器,这里不用管
    while (co == nullptr) {
      std::this_thread::sleep_for(2ms);
      co = scheduler_.ScheduleOne();
    }

    co->Resume();
    if (co->Finished()) {
      int ret = co->GetRet().GetRetCode();
      CoroAllocator::GetInst().Free(co);
      if (co->GetID() == CoroAllocator::main_id) {
        return ret;
      }
    }
  }
  return 0;
}

} // namespace ab
