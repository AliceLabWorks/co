#pragma once
#include "corotine.h"
#include "singleton.h"
#include <memory>
#include <unordered_map>

namespace ab {
class CoroAllocator : public Singleton<CoroAllocator> {
public:
  static constexpr uint64_t main_id = 1;
  CoroAllocator() = default;
  Corotine *Alloc() {
    static uint64_t id = 0;
    id++;
    coro_map_[id] = std::unique_ptr<Corotine>(new Corotine(id));
    return coro_map_[id].get();
  }

  void Free(Corotine *coro) {
    auto iter = coro_map_.find(coro->GetID());
    if (iter != coro_map_.end()) {
      coro_map_.erase(iter);
    }
  }

  Corotine *Find(uint64_t id) {
    auto iter = coro_map_.find(id);
    if (iter == coro_map_.end()) {
      return nullptr;
    }
    return iter->second.get();
  }

private:
  std::unordered_map<uint64_t, std::unique_ptr<Corotine>> coro_map_;
};
}; // namespace ab