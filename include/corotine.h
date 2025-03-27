#pragma once

#include "boost/context/fiber.hpp"
#include "this_coro.h"

namespace ctx = boost::context;

namespace ab {
using CoFunc = std::function<void()>;

class Corotine;

class CoRet {
public:
  friend class Corotine;
  int32_t GetRetCode() const { return ret_code; }
  std::string GetRetMsg() const { return ret_msg; }

private:
  int32_t ret_code = 0;
  std::string ret_msg;
};

class Corotine {
public:
  Corotine(uint64_t id) : id_(id) {}

  template <typename Func> void Init(Func &&co_func);
  void Resume();
  void Yield();

  bool Finished() const;
  CoRet GetRet() const;
  uint64_t GetID() const;
  void SetRetVal(int val);
  void SetRetMsg(const std::string &msg);

private:
  ctx::fiber fiber_;
  ctx::fiber main_;
  uint64_t id_;
  CoRet ret_;
};

template <typename Func> void Corotine::Init(Func &&co_func) {
  static_assert(std::is_constructible_v<CoFunc, Func &&>,
                "Func must be constructible to CoFunc");
  fiber_ = ctx::fiber(
      [this, co_func_ = std::forward<Func>(co_func)](ctx::fiber &&f) mutable {
        main_ = std::move(f);
        auto &thread_info = ThisCoro::GetThreadInfo();
        assert(thread_info.async_safe_point == false);
        thread_info.async_safe_point = true;
        co_func_();
        assert(thread_info.async_safe_point == true);
        thread_info.async_safe_point = false;
        return std::move(main_);
      });
}
} // namespace ab
