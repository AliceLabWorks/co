#pragma once

#include "colang_server.h"
#include "signal_handler.h"

#define CO_MAIN()                                                              \
  void main_func();                                                            \
  int main(int argc, char **argv) {                                            \
    ab::InstallFailureSignalHandler();                                         \
    return ab::ColangServer::GetInst().Start(main_func);                       \
  }                                                                            \
  void main_func()

#define CO_RETUN(ret_val)                                                      \
  ab::ThisCoro::SetRet(ret_val);                                               \
  return;
