#ifndef KERNEL_DRIVER_H
#define KERNEL_DRIVER_H

#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <string.h>
#include <vector>

#define HMKPM_MAGIC_READ  0x484D4B504EULL
#define HMKPM_MAGIC_WRITE 0x484D4B504FULL

struct hmkpm_req {
    uint64_t addr;
    uint64_t size;
    uint32_t pid;
    uint32_t _pad;
};

class KernelDriver {
public:
    static bool isLoaded() {
        // Quick read to test if the KPM is intercepting the syscall
        uint64_t dummy = 0;
        return readMemory(getpid(), (uint64_t)&dummy, &dummy, sizeof(dummy));
    }

    static bool readMemory(pid_t pid, uint64_t addr, void *buffer, size_t size) {
        if (size > 0x100000) return false;

        // Allocate memory for the header + the payload
        size_t total_size = sizeof(hmkpm_req) + size;
        std::vector<uint8_t> payload(total_size);

        struct hmkpm_req* req = (struct hmkpm_req*)payload.data();
        req->addr = addr;
        req->size = size;
        req->pid = pid;
        req->_pad = 0;

        // The data buffer comes right after the 24-byte header
        void* data_ptr = payload.data() + sizeof(hmkpm_req);

        // Syscall: __NR_getresuid(magic, user_buffer, total_size)
        long ret = syscall(__NR_getresuid, HMKPM_MAGIC_READ, payload.data(), total_size);
        
        if (ret == 0) {
            memcpy(buffer, data_ptr, size);
            return true;
        }
        return false;
    }

    static bool writeMemory(pid_t pid, uint64_t addr, const void *buffer, size_t size) {
        if (size > 0x100000) return false;

        size_t total_size = sizeof(hmkpm_req) + size;
        std::vector<uint8_t> payload(total_size);

        struct hmkpm_req* req = (struct hmkpm_req*)payload.data();
        req->addr = addr;
        req->size = size;
        req->pid = pid;
        req->_pad = 0;

        // Copy the user data into the payload right after the header
        void* data_ptr = payload.data() + sizeof(hmkpm_req);
        memcpy(data_ptr, buffer, size);

        // Syscall
        long ret = syscall(__NR_getresuid, HMKPM_MAGIC_WRITE, payload.data(), total_size);
        
        return ret == 0;
    }
};

#endif
