#include "co_macro.h"
#include "colang_server.h"
#include <iostream>

using namespace ab;

CO_MAIN() {
  co([]() {
    for (int i = 3000000; i < 6000000; i++) {
      int jj = 0;
      for (int tmp = 0; tmp < 10000; tmp++) {
        jj++;
      }
      if (i % 1000 == 0) {
        AsyncSafeGuard guard;
        std::cout << i << " co1" << std::endl;
      }
    }
  })();
  std::cout << "hello world" << std::endl;
  for (int i = 0; i < 3000000; i++) {
    int jj = 0;
    for (int tmp = 0; tmp < 10000; tmp++) {
      jj++;
    }
    if (i % 1000 == 0) {
      AsyncSafeGuard guard;
      std::cout << i << " co2" << std::endl;
    }
  }

  std::cout << "bye world" << std::endl;
  CO_RETUN(0);
}