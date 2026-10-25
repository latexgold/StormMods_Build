#ifndef KERNEL_DRIVER_H
#define KERNEL_DRIVER_H

#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

// HMKPM Magic Commands
#define HMKPN_READ_CMD  0x484D4B504EULL // "HMKPN"
#define HMKPO_WRITE_CMD 0x484D4B504FULL // "HMKPO"

// The structure HMKPM expects
struct hmkpm_rw {
    uint64_t magic;
    pid_t pid;
    uint64_t addr;
    void* buffer;
    uint64_t size;
};

class KernelDriver {
public:
    static bool isLoaded() {
        // A quick dummy read to see if the syscall is intercepted
        uint64_t dummy = 0;
        struct hmkpm_rw rw;
        rw.magic = HMKPN_READ_CMD;
        rw.pid = getpid();
        rw.addr = (uint64_t)&dummy;
        rw.buffer = &dummy;
        rw.size = sizeof(dummy);
        
        // __NR_getresuid is syscall 148 on arm64. 
        // HMKPM intercepts this and checks the arguments.
        long ret = syscall(__NR_getresuid, &rw, nullptr, nullptr);
        return ret == 0;
    }

    static bool readMemory(pid_t pid, uint64_t addr, void *buffer, size_t size) {
        struct hmkpm_rw rw;
        rw.magic = HMKPN_READ_CMD;
        rw.pid = pid;
        rw.addr = addr;
        rw.buffer = buffer;
        rw.size = size;
        
        long ret = syscall(__NR_getresuid, &rw, nullptr, nullptr);
        return ret == 0;
    }

    static bool writeMemory(pid_t pid, uint64_t addr, const void *buffer, size_t size) {
        struct hmkpm_rw rw;
        rw.magic = HMKPO_WRITE_CMD;
        rw.pid = pid;
        rw.addr = addr;
        rw.buffer = const_cast<void*>(buffer);
        rw.size = size;
        
        long ret = syscall(__NR_getresuid, &rw, nullptr, nullptr);
        return ret == 0;
    }
};

#endif
