#ifndef UTILS
#define UTILS

#include <jni.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <fstream>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include "Logger.h"
#include "KernelDriver.h"

pid_t target_pid = -1;
uintptr_t libAddress, anogs;
extern bool g_useKernelDriver;
bool g_useKernelDriver = false;

// Define os syscalls corretamente conforme a arquitetura
#if defined(__aarch64__)
// 64-bit ARM
#define SYS_process_vm_readv 270
#define SYS_process_vm_writev 271
#define SYS_openat 56
#elif defined(__x86_64__)
// 64-bit x86
    #define SYS_process_vm_readv 310
    #define SYS_process_vm_writev 311
    #define SYS_open 2
#elif defined(__arm__)
    // 32-bit ARM
    #define SYS_process_vm_readv 376
    #define SYS_process_vm_writev 377
    #define SYS_open 5
#elif defined(__i386__)
    // 32-bit x86
    #define SYS_process_vm_readv 347
    #define SYS_process_vm_writev 348
    #define SYS_open 5
#else
    #error "Arquitetura não suportada"
#endif

#define halfShift 10
#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD

typedef char UTF8;
typedef unsigned short UTF16;
typedef unsigned int UTF32;
typedef unsigned char uint8;
typedef unsigned int uint32;
static const UTF32 halfBase = 0x0010000UL;
static const UTF8 firstByteMark[7] = { 0x00, 0x00, static_cast<UTF8>(0xC0), static_cast<UTF8>(0xE0), static_cast<UTF8>(0xF0), static_cast<UTF8>(0xF8), static_cast<UTF8>(0xFC) };

// Encontra o PID do processo pelo nome do pacote
pid_t FindPid(const char *PackageName) {
    char text[69];
    pid_t pid = 0;
    sprintf(text, "pidof %s", PackageName);
    FILE *chkRun = popen(text, "r");
    if (chkRun) {
        char output[10];
        if (fgets(output, sizeof(output), chkRun)) {
            pid = atoi(output);
        }
        pclose(chkRun);
    }
    return pid;
}

// Encontra o endereço base de uma biblioteca dentro do processo alvo
uintptr_t FindLibrary(const char* name, int index) {
    int i = 0;
    uintptr_t start = 0;
    char line[1024] = {0};
    char dname[128], fname[128];

    snprintf(dname, sizeof(dname), "%s", name);
    snprintf(fname, sizeof(fname), "/proc/%d/maps", target_pid);
    FILE* p = fopen(fname, "r");
    if (p) {
        while (fgets(line, sizeof(line), p)) {
            if (strstr(line, dname) != NULL) {
                i++;
                if (i == index) {
                    sscanf(line, "%" SCNxPTR, &start);
                    break;
                }
            }
        }
        fclose(p);
    }
    return start;
}

// Lê memória usando process_vm_readv
/*template<class T>
T Read(uintptr_t address) {
    T buf{};
    struct iovec local, remote;

    local.iov_base = &buf;
    local.iov_len = sizeof(T);
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = sizeof(T);

    ssize_t bytes = syscall(SYS_process_vm_readv, target_pid, &local, 1, &remote, 1, 0);
    if (bytes != sizeof(T)) {
        // Error handling - você pode querer lançar uma exceção ou logar erro
        memset(&buf, 0, sizeof(T));
    }
    return buf;
}*/
template<class T>
T Read(uintptr_t address) {
    T buf{};
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        if (KernelDriver::readMemory(target_pid, address, &buf, sizeof(T))) {
            return buf;
        }
        g_useKernelDriver = false;
    }
    struct iovec local, remote;
    local.iov_base = &buf;
    local.iov_len = sizeof(T);
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = sizeof(T);
    ssize_t bytes = syscall(SYS_process_vm_readv, target_pid, &local, 1, &remote, 1, 0);
    if (bytes != sizeof(T)) {
        memset(&buf, 0, sizeof(T));
    }
    return buf;
}

// Especialização para ler strings
template<>
std::string Read<std::string>(uintptr_t address) {
    std::string result;
    const size_t max_len = 4096;
    char buffer[max_len] = {0};

    struct iovec local, remote;
    local.iov_base = buffer;
    local.iov_len = max_len - 1; // Leave room for null terminator
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = max_len - 1;

    ssize_t bytes = syscall(SYS_process_vm_readv, target_pid, &local, 1, &remote, 1, 0);
    if (bytes > 0) {
        buffer[bytes < max_len ? bytes : max_len - 1] = '\0';
        result = buffer;
    }

    return result;
}

// Escreve memória usando process_vm_writev
/*template<typename T>
bool Write(uintptr_t address, T value) {
    struct iovec local, remote;

    local.iov_base = &value;
    local.iov_len = sizeof(T);
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = sizeof(T);

    ssize_t bytes = syscall(SYS_process_vm_writev, target_pid, &local, 1, &remote, 1, 0);
    return (bytes == sizeof(T));
}*/
template<typename T>
bool Write(uintptr_t address, T value) {
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        if (KernelDriver::writeMemory(target_pid, address, &value, sizeof(T))) {
            return true;
        }
        g_useKernelDriver = false;
    }
    struct iovec local, remote;
    local.iov_base = &value;
    local.iov_len = sizeof(T);
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = sizeof(T);
    ssize_t bytes = syscall(SYS_process_vm_writev, target_pid, &local, 1, &remote, 1, 0);
    return (bytes == sizeof(T));
}

uintptr_t getMemoryAddr(uintptr_t address) {
    return Read<uintptr_t>(address);
}

__attribute__((always_inline))
static inline int my_open_mem(int pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/mem";

#if defined(__aarch64__)
    return syscall(SYS_openat, AT_FDCWD, path.c_str(), O_RDWR, 0);
#elif defined(__x86_64__)
    return syscall(SYS_open, path.c_str(), O_RDWR);
#elif defined(__arm__) || defined(__i386__)
    return syscall(SYS_open, path.c_str(), 00000002);
#else
    return -1;
#endif
}

int mem_fd = 0;

std::vector<uint8_t> ParseHexBytes(const std::string& hexString) {
    std::vector<uint8_t> bytes;
    std::istringstream stream(hexString);
    std::string byteStr;

    while (stream >> byteStr) {
        try {
            bytes.push_back(static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16)));
        } catch (...) {
            // Ignore invalid bytes
        }
    }
    return bytes;
}

bool WriteIO(uintptr_t address, const std::vector<uint8_t>& bytes) {
    if (mem_fd <= 0) {
        mem_fd = my_open_mem(target_pid);
        if (mem_fd <= 0) return false;
    }

    ssize_t written = pwrite(mem_fd, bytes.data(), bytes.size(), static_cast<off_t>(address));
    return (written == static_cast<ssize_t>(bytes.size()));
}
bool WriteIO1(uintptr_t address, const void* data, size_t size) {
    if (mem_fd <= 0) {
        mem_fd = my_open_mem(target_pid);
        if (mem_fd <= 0) return false;
    }
    ssize_t written = pwrite(mem_fd, data, size, static_cast<off_t>(address));
    return (written == static_cast<ssize_t>(size));
}

bool WriteIO(uintptr_t address, const std::string& hexString) {
    std::vector<uint8_t> bytes = ParseHexBytes(hexString);
    return WriteIO(address, bytes);
}

struct D3DMatrix {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
};

struct Vector4 {
    float X;
    float Y;
    float Z;
    float W;
};

class MonoDictionary {
public:
    uintptr_t getValues() {
        return Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x18) + 0x20; //0x18
    }

    int getNumValues() {
        return Read<int>(reinterpret_cast<uintptr_t>(this) + 0x20); //0x24
    }
};

uintptr_t string2Offset(const char *c) {
    int base = 16;

    // Safe conversion for all architectures
    std::string str(c);
    if (str.empty()) return 0;

    // Remove "0x" prefix if present
    if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str = str.substr(2);
    }

    uintptr_t result = 0;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> result;

    return result;
}

typedef enum {
    strictConversion = 0,
    lenientConversion
} ConversionFlags;

typedef enum {
    conversionOK,        /* conversion successful */
    sourceExhausted,     /* partial character in source, but hit end */
    targetExhausted,     /* insuff. room in target for conversion */
    sourceIllegal,       /* source sequence is illegal/malformed */
    conversionFailed
} ConversionResult;

int Utf16_To_Utf8(const UTF16 *sourceStart, UTF8 *targetStart, size_t outLen,
                  ConversionFlags flags) {
    int result = conversionOK;
    const UTF16 *source = sourceStart;
    UTF8 *target = targetStart;
    UTF8 *targetEnd = targetStart + outLen;

    if ((source == nullptr) || (targetStart == nullptr)) {
        return conversionFailed;
    }

    while (*source) {
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        const UTF32 byteMask = 0xBF;
        const UTF32 byteMark = 0x80;
        const UTF16 *oldSource = source;

        ch = *source++;

        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            if (*source) {
                UTF32 ch2 = *source;
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                } else if (flags == strictConversion) {
                    --source;
                    result = sourceIllegal;
                    break;
                }
            } else {
                --source;
                result = sourceExhausted;
                break;
            }
        } else if (flags == strictConversion) {
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
                --source;
                result = sourceIllegal;
                break;
            }
        }

        if (ch < (UTF32)0x80) {
            bytesToWrite = 1;
        } else if (ch < (UTF32)0x800) {
            bytesToWrite = 2;
        } else if (ch < (UTF32)0x10000) {
            bytesToWrite = 3;
        } else if (ch < (UTF32)0x110000) {
            bytesToWrite = 4;
        } else {
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        if (target > targetEnd) {
            source = oldSource;
            target -= bytesToWrite;
            result = targetExhausted;
            break;
        }

        switch (bytesToWrite) {
            case 4:
                *--target = (UTF8)((ch | byteMark) & byteMask);
                ch >>= 6;
                [[fallthrough]];
            case 3:
                *--target = (UTF8)((ch | byteMark) & byteMask);
                ch >>= 6;
                [[fallthrough]];
            case 2:
                *--target = (UTF8)((ch | byteMark) & byteMask);
                ch >>= 6;
                [[fallthrough]];
            case 1:
                *--target = (UTF8)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }

    // Null terminate if there's space
    if (target < targetEnd) {
        *target = '\0';
    } else if (target == targetEnd) {
        *(target - 1) = '\0';
    }

    return result;
}

void getCharacterName(uintptr_t address, UTF8 *transcoding, size_t bufSize) {
    if (bufSize < 1) return;

    memset(transcoding, 0, bufSize);

    UTF16 buf16[34] = {0};
    int m = 0;

    for (int i = 0; i < 4; i++) {
        int classname = Read<int>(address + i * 4);
        int high = (classname >> 16) & 0xFFFF;
        int low = classname & 0xFFFF;

        if (low) buf16[m++] = static_cast<UTF16>(low);
        if (high) buf16[m++] = static_cast<UTF16>(high);
        if (m >= 32) break; // Prevent buffer overflow
    }

    buf16[m] = 0; // Null terminate UTF16 string

    int result = Utf16_To_Utf8(buf16, transcoding, bufSize - 1, strictConversion);
    if (result != conversionOK) {
        // Fallback to empty string
        transcoding[0] = '\0';
    }
}

// Helper functions for common operations
bool WriteBytes(uintptr_t address, const void* data, size_t size) {
    if (size == 0) return true;

    struct iovec local, remote;
    local.iov_base = const_cast<void*>(data);
    local.iov_len = size;
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = size;

    ssize_t bytes = syscall(SYS_process_vm_writev, target_pid, &local, 1, &remote, 1, 0);
    return (bytes == static_cast<ssize_t>(size));
}

bool ReadBytes(uintptr_t address, void* buffer, size_t size) {
    if (size == 0 || buffer == nullptr) return false;

    struct iovec local, remote;
    local.iov_base = buffer;
    local.iov_len = size;
    remote.iov_base = reinterpret_cast<void*>(address);
    remote.iov_len = size;

    ssize_t bytes = syscall(SYS_process_vm_readv, target_pid, &local, 1, &remote, 1, 0);
    return (bytes == static_cast<ssize_t>(size));
}

// Initialize the memory file descriptor
bool InitMemoryAccess(pid_t pid) {
    target_pid = pid;
    mem_fd = my_open_mem(pid);
    return (mem_fd > 0);
}

// Cleanup
void CleanupMemoryAccess() {
    if (mem_fd > 0) {
        close(mem_fd);
        mem_fd = 0;
    }
    target_pid = -1;
}

#endif // UTILS
