#ifndef KERNEL_DRIVER_H
#define KERNEL_DRIVER_H

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <string.h>

// HMKPM uses PRCTL for kernel communication
#define KPM_PRCTL_CMD 0xDEADBEEF
#define KPM_OP_READ   1
#define KPM_OP_WRITE  2

struct kpm_mem_rw {
    pid_t pid;
    uint64_t addr;
    void* buffer;
    uint64_t size;
    int is_write;
};

class KernelDriver {
public:
    static bool isLoaded() {
        // Try to read a null pointer. If it doesn't crash, the KPM is loaded.
        uint64_t test = 0;
        struct kpm_mem_rw rw;
        rw.pid = getpid();
        rw.addr = (uint64_t)&test;
        rw.buffer = &test;
        rw.size = sizeof(test);
        rw.is_write = 0;
        // A return of 0 or >0 usually means the KPM intercepted it
        return prctl(KPM_PRCTL_CMD, KPM_OP_READ, &rw) >= 0;
    }

    static bool readMemory(pid_t pid, uint64_t addr, void *buffer, size_t size) {
        struct kpm_mem_rw rw;
        rw.pid = pid;
        rw.addr = addr;
        rw.buffer = buffer;
        rw.size = size;
        rw.is_write = 0;
        return prctl(KPM_PRCTL_CMD, KPM_OP_READ, &rw) == 0;
    }

    static bool writeMemory(pid_t pid, uint64_t addr, const void *buffer, size_t size) {
        struct kpm_mem_rw rw;
        rw.pid = pid;
        rw.addr = addr;
        rw.buffer = const_cast<void*>(buffer);
        rw.size = size;
        rw.is_write = 1;
        return prctl(KPM_PRCTL_CMD, KPM_OP_WRITE, &rw) == 0;
    }
};

#endif
