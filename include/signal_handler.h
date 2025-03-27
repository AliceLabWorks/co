#pragma once
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
extern sigset_t old_mask;
extern int InstallFailureSignalHandler();
} // namespace ab