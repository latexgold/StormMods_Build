#ifndef KERNEL_DRIVER_H
#define KERNEL_DRIVER_H

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <android/log.h>
#include <stdint.h>
#include <arpa/inet.h>

// The command defined in the KPM
#define OP_READ_MEM 8001

// This struct MUST exactly match the struct in the KPM's hello.c
struct copy_memory_t {
    pid_t pid;
    uintptr_t addr;
    void* buffer;
    size_t size;
};

class KernelDriver {
public:
    static bool isLoaded() {
        // The KPM hooks ioctl on UDP sockets. If we can create one, the KPM is active.
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd >= 0) {
            close(fd);
            return true;
        }
        return false;
    }

    static bool readMemory(pid_t pid, uint64_t addr, void *buffer, size_t size) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) return false;

        struct copy_memory_t rw;
        rw.pid = pid;
        rw.addr = addr;
        rw.buffer = buffer;
        rw.size = size;

        // Send the read request to the KPM hook
        int ret = ioctl(fd, OP_READ_MEM, &rw);
        close(fd);
        
        // KPM returns 0 on success
        return ret == 0;
    }

    static bool writeMemory(pid_t pid, uint64_t addr, const void *buffer, size_t size) {
        // The linked KPM only implements OP_READ_MEM (8001).
        // It does not have a write function. Return false so the hack 
        // knows it cannot write memory safely.
        return false; 
    }
};

#endif
