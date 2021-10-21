#include "syscall.h"
#include "time.h"

static inline int64_t syscall(uint64_t id, uint64_t a0, uint64_t a1,
                              uint64_t a2) {
  int64_t ret;
  asm volatile("mv x10, %1\n"
               "mv x11, %2\n"
               "mv x12, %3\n"
               "mv x17, %4\n"
               "ecall\n"
               "mv %0, x10\n"
               : "=r"(ret)
               : "r"(a0), "r"(a1), "r"(a2), "r"(id)
               : "memory", "x10", "x11", "x12", "x17");
  return ret;
}

int64_t write(uint64_t fd, char *buf, uint64_t len) {
  return syscall(SYSCALL_WRITE, fd, (uint64_t)buf, len);
}

int64_t exit(int exit_code) {
  return syscall(SYSCALL_EXIT, (uint64_t)exit_code, 0, 0);
}

int64_t yield() { return syscall(SYSCALL_YIELD, 0, 0, 0); }

int64_t set_priority(int64_t prio) {
  return syscall(SYSCALL_SET_PRIORITY, (uint64_t)prio, 0, 0);
}

int64_t get_time() {
  TimeVal time;
  if (syscall(SYSCALL_GET_TIME, (uint64_t)&time, 0, 0) == 0) {
    return (int64_t)((time.sec & 0xffff) * 1000 + time.usec / 1000);
  }
  return -1;
}

int64_t mmap(uint64_t start, uint64_t len, uint64_t prot) {
  return syscall(SYSCALL_MMAP, start, len, prot);
}

int64_t munmap(uint64_t start, uint64_t len) {
  return syscall(SYSCALL_MUNMAP, start, len, 0);
}
