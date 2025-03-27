
#include "colang_server.h"
#include "this_coro.h"
#include <atomic>
#include <cxxabi.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>

namespace ab {
const struct {
  int number;
  const char *name;
} kFailureSignals[] = {
    {SIGSEGV, "SIGSEGV"}, {SIGILL, "SIGILL"}, {SIGFPE, "SIGFPE"},
    {SIGABRT, "SIGABRT"}, {SIGBUS, "SIGBUS"}, {SIGTERM, "SIGTERM"},
};

const struct {
  int number;
  const char *name;
} kUsrSignals[] = {
    {SIGUSR1, "SIGUSR1"},
};

extern "C" void AsyncYieldCoro();

void FailureSignalHandler(int signal_number, siginfo_t *signal_info,
                          void *ucontext) {
  if (ThisCoro::IsAsyncSafePoint() == false) {
    return;
  }
  ThisCoro::GetThreadInfo().handling_signal = true;
  // 阻塞SIGUSR1信号，防止重入
  sigset_t mask, old_mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  pthread_sigmask(SIG_BLOCK, &mask, &old_mask);

  ucontext_t *ucontext_ptr = (ucontext_t *)ucontext;
  // 获取当前栈指针
  void *stack_ptr = (void *)ucontext_ptr->uc_mcontext.gregs[REG_RSP];
  // 将栈指针向下移动8字节（64位系统）
  stack_ptr = (void *)((char *)stack_ptr - sizeof(void *));
  ucontext_ptr->uc_mcontext.gregs[REG_RSP] = (greg_t)stack_ptr;
  // 将要执行的函数地址写入栈中
  *(greg_t *)stack_ptr = ucontext_ptr->uc_mcontext.gregs[REG_RIP];
  ucontext_ptr->uc_mcontext.gregs[REG_RIP] = (greg_t)&AsyncYieldCoro;
  pthread_sigmask(SIG_SETMASK, &old_mask, NULL); // 恢复信号掩码
}

int InstallFailureSignalHandler() {
  // Build the sigaction struct.
  struct sigaction sig_action;
  memset(&sig_action, 0, sizeof(sig_action));
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags |= SA_SIGINFO;
  sig_action.sa_sigaction = &FailureSignalHandler;

  for (size_t i = 0; i < sizeof(kFailureSignals); ++i) {
    int ret = sigaction(kUsrSignals[i].number, &sig_action, NULL);
    if (ret != 0) {
      return -1;
    }
  }
  return 0;
}
} // namespace ab
