.global AsyncYieldCoro
.type AsyncYieldCoro, @function

AsyncYieldCoro:
    pushq %rbp
    movq %rsp, %rbp

    pushfq

    subq $368, %rsp

    movq %rax, 0(%rsp)
    movq %rcx, 8(%rsp)
    movq %rdx, 16(%rsp)
    movq %rbx, 24(%rsp)
    movq %rsi, 32(%rsp)
    movq %rdi, 40(%rsp)
    movq %r8, 48(%rsp)
    movq %r9, 56(%rsp)
    movq %r10, 64(%rsp)
    movq %r11, 72(%rsp)
    movq %r12, 80(%rsp)
    movq %r13, 88(%rsp)
    movq %r14, 96(%rsp)
    movq %r15, 104(%rsp)

    movups %xmm0, 112(%rsp)
    movups %xmm1, 128(%rsp)
    movups %xmm2, 144(%rsp)
    movups %xmm3, 160(%rsp)
    movups %xmm4, 176(%rsp)
    movups %xmm5, 192(%rsp)
    movups %xmm6, 208(%rsp)
    movups %xmm7, 224(%rsp)
    movups %xmm8, 240(%rsp)
    movups %xmm9, 256(%rsp)
    movups %xmm10, 272(%rsp)
    movups %xmm11, 288(%rsp)
    movups %xmm12, 304(%rsp)
    movups %xmm13, 320(%rsp)
    movups %xmm14, 336(%rsp)
    movups %xmm15, 352(%rsp)

    # 调用协程让出函数
    call YieldBySignal

    movups 352(%rsp), %xmm15
    movups 336(%rsp), %xmm14
    movups 320(%rsp), %xmm13
    movups 304(%rsp), %xmm12
    movups 288(%rsp), %xmm11
    movups 272(%rsp), %xmm10
    movups 256(%rsp), %xmm9
    movups 240(%rsp), %xmm8
    movups 224(%rsp), %xmm7
    movups 208(%rsp), %xmm6
    movups 192(%rsp), %xmm5
    movups 176(%rsp), %xmm4
    movups 160(%rsp), %xmm3
    movups 144(%rsp), %xmm2
    movups 128(%rsp), %xmm1
    movups 112(%rsp), %xmm0

    movq 104(%rsp), %r15
    movq 96(%rsp), %r14
    movq 88(%rsp), %r13
    movq 80(%rsp), %r12
    movq 72(%rsp), %r11
    movq 64(%rsp), %r10
    movq 56(%rsp), %r9
    movq 48(%rsp), %r8
    movq 40(%rsp), %rdi
    movq 32(%rsp), %rsi
    movq 24(%rsp), %rbx
    movq 16(%rsp), %rdx
    movq 8(%rsp), %rcx
    movq 0(%rsp), %rax

    addq $368, %rsp

    popfq
    popq %rbp
    ret
