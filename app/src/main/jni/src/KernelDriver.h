#ifndef KERNEL_DRIVER_H
#define KERNEL_DRIVER_H

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <android/log.h>

#define NH_DEVICE "/dev/nh"

struct nh_rw {
    pid_t pid;
    uint64_t addr;
    void *data;
    size_t size;
};

#define NH_READ   _IOWR('N', 1, struct nh_rw)
#define NH_WRITE  _IOWR('N', 2, struct nh_rw)

class KernelDriver {
public:
    static bool isLoaded() {
        int fd = open(NH_DEVICE, O_RDWR);
        if (fd >= 0) {
            close(fd);
            return true;
        }
        return false;
    }

    static bool readMemory(pid_t pid, uint64_t addr, void *buffer, size_t size) {
        int fd = open(NH_DEVICE, O_RDWR);
        if (fd < 0) return false;

        struct nh_rw rw;
        rw.pid = pid;
        rw.addr = addr;
        rw.data = buffer;
        rw.size = size;

        int ret = ioctl(fd, NH_READ, &rw);
        close(fd);
        return ret == 0;
    }

    static bool writeMemory(pid_t pid, uint64_t addr, const void *buffer, size_t size) {
        int fd = open(NH_DEVICE, O_RDWR);
        if (fd < 0) return false;

        struct nh_rw rw;
        rw.pid = pid;
        rw.addr = addr;
        rw.data = const_cast<void*>(buffer);
        rw.size = size;

        int ret = ioctl(fd, NH_WRITE, &rw);
        close(fd);
        return ret == 0;
    }
};

#endif