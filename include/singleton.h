#pragma once
template <typename T> class Singleton {
public:
  static T &GetInst() {
    static T instance;
    return instance;
  }
  ~Singleton() {}

protected:
  Singleton() {}
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;
};
