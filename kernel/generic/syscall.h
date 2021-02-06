#pragma once
#include <lock.h>
#include <stdint.h>
#include <utils/syscall_codes.h>
typedef long int time_t;
struct timespec
{
    time_t tv_sec; /* time in seconds */
    long tv_nsec;  /* time in nanoseconds */
};
extern lock_type lck_syscall;

void init_syscall();
uint64_t syscall(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, InterruptStackFrame *stackframe);
