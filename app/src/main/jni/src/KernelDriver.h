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

#define OP_READ_MEM 8001

struct copy_memory_t {
    uint32_t pid;
    uint32_t _pad0;
    uint64_t addr;
    uint64_t buffer;
    uint64_t size;
};

class KernelDriver {
public:
    static bool isLoaded() {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd >= 0) { close(fd); return true; }
        return false;
    }

    static bool readMemory(pid_t pid, uint64_t addr, void *buffer, size_t size) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) return false;
        struct copy_memory_t rw;
        rw.pid = pid;
        rw._pad0 = 0;
        rw.addr = addr;
        rw.buffer = (uint64_t)buffer;
        rw.size = size;
        int ret = ioctl(fd, OP_READ_MEM, &rw);
        close(fd);
        return ret == 0;
    }

    static bool writeMemory(pid_t pid, uint64_t addr, const void *buffer, size_t size) {
        // Forces fallback to the Anti-Detect Micro-Write in Utils.h
        return false; 
    }
};

#endif
