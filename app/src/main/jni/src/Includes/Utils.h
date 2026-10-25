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
#include "Logger.h"
#include "KernelDriver.h"

pid_t target_pid = -1;
uintptr_t libAddress, anogs;
extern bool g_useKernelDriver;
bool g_useKernelDriver = true;

int mem_fd = 0;
int my_open_mem(int pid) { return 0; }

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

pid_t FindPid(const char *PackageName) {
    char text[69];
    pid_t pid = 0;
    sprintf(text, "pidof %s", PackageName);
    FILE *chkRun = popen(text, "r");
    if (chkRun) {
        char output[10];
        if (fgets(output, sizeof(output), chkRun)) { pid = atoi(output); }
        pclose(chkRun);
    }
    return pid;
}

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
                if (i == index) { sscanf(line, "%" SCNxPTR, &start); break; }
            }
        }
        fclose(p);
    }
    return start;
}

template<class T>
T Read(uintptr_t address) {
    T buf{};
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        if (KernelDriver::readMemory(target_pid, address, &buf, sizeof(T))) {
            return buf;
        }
    }
    return buf; 
}

template<>
std::string Read<std::string>(uintptr_t address) {
    std::string result;
    const size_t max_len = 4096;
    char buffer[max_len] = {0};
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        if (KernelDriver::readMemory(target_pid, address, buffer, max_len - 1)) {
            buffer[max_len - 1] = '\0';
            result = buffer;
        }
    }
    return result;
}

template<typename T>
bool Write(uintptr_t address, T value) {
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        return KernelDriver::writeMemory(target_pid, address, &value, sizeof(T));
    }
    return false;
}

uintptr_t getMemoryAddr(uintptr_t address) { return Read<uintptr_t>(address); }

bool WriteIO(uintptr_t address, const std::vector<uint8_t>& bytes) {
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        return KernelDriver::writeMemory(target_pid, address, bytes.data(), bytes.size());
    }
    return false;
}

bool WriteIO1(uintptr_t address, const void* data, size_t size) {
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        return KernelDriver::writeMemory(target_pid, address, data, size);
    }
    return false;
}

// The rest of the Utils.h (D3DMatrix, Vector4, MonoDictionary, string converters, etc.)
// remains exactly as it was in the original code. 
