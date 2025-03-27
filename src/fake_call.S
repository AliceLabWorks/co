.global AsyncYieldCoro
AsyncYieldCoro:
    # 保存调用者需要保存的寄存器
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11

    # 调用协程让出函数
    call YieldBySignal

    # 恢复调用者需要保存的寄存器
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rax
    ret
