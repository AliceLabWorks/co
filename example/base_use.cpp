#include "co_macro.h"
#include "colang_server.h"
#include <iostream>

using namespace ab;

CO_MAIN() {
  co([]() {
    for (int i = 3; i < 6; i++) {
      std::cout << i << " world" << std::endl;
      ThisCoro::Yield();
    }
  })();
  std::cout << "hello world" << std::endl;
  for (int i = 0; i < 3; i++) {
    std::cout << i << " world" << std::endl;
    ThisCoro::Yield();
  }

  ColangServer::GetInst().AddTimer(
      []() { std::cout << "i am timer" << std::endl; }, 20);

  std::cout << "start sleep" << std::endl;
  ThisCoro::Sleep(5000);
  std::cout << "bye world" << std::endl;
  CO_RETUN(0);
}