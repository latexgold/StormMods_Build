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

// Dummy variables to fix Server.cpp compilation
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

// READ VIA HMKPM (Ring-0)
template<class T>
T Read(uintptr_t address) {
    T buf{};
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        if (KernelDriver::readMemory(target_pid, address, &buf, sizeof(T))) {
            return buf;
        }
    }
    return buf; // Fails silently if KPM is not loaded
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

// WRITE VIA HMKPM (Ring-0)
template<typename T>
bool Write(uintptr_t address, T value) {
    if (g_useKernelDriver && KernelDriver::isLoaded()) {
        return KernelDriver::writeMemory(target_pid, address, &value, sizeof(T));
    }
    return false;
}

uintptr_t getMemoryAddr(uintptr_t address) { return Read<uintptr_t>(address); }

std::vector<uint8_t> ParseHexBytes(const std::string& hexString) {
    std::vector<uint8_t> bytes;
    std::istringstream stream(hexString);
    std::string byteStr;
    while (stream >> byteStr) {
        try { bytes.push_back(static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16))); } catch (...) {}
    }
    return bytes;
}

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

bool WriteIO(uintptr_t address, const std::string& hexString) {
    return WriteIO(address, ParseHexBytes(hexString));
}

// --- REQUIRED CLASSES AND STRUCTS FOR SERVER.CPP ---
struct D3DMatrix { float _11, _12, _13, _14, _21, _22, _23, _24, _31, _32, _33, _34, _41, _42, _43, _44; };
struct Vector4 { float X, Y, Z, W; };

class MonoDictionary {
public:
    uintptr_t getValues() { return Read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x18) + 0x20; }
    int getNumValues() { return Read<int>(reinterpret_cast<uintptr_t>(this) + 0x20); }
};

typedef enum { strictConversion = 0, lenientConversion } ConversionFlags;
typedef enum { conversionOK, sourceExhausted, targetExhausted, sourceIllegal, conversionFailed } ConversionResult;

int Utf16_To_Utf8(const UTF16 *sourceStart, UTF8 *targetStart, size_t outLen, ConversionFlags flags) {
    int result = conversionOK;
    const UTF16 *source = sourceStart;
    UTF8 *target = targetStart;
    UTF8 *targetEnd = targetStart + outLen;
    if (!source || !targetStart) return conversionFailed;
    while (*source) {
        UTF32 ch; unsigned short bytesToWrite = 0;
        const UTF32 byteMask = 0xBF, byteMark = 0x80;
        const UTF16 *oldSource = source;
        ch = *source++;
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            if (*source) {
                UTF32 ch2 = *source;
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase; ++source;
                } else if (flags == strictConversion) { --source; result = sourceIllegal; break; }
            } else { --source; result = sourceExhausted; break; }
        } else if (flags == strictConversion) {
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) { --source; result = sourceIllegal; break; }
        }
        if (ch < 0x80) bytesToWrite = 1; else if (ch < 0x800) bytesToWrite = 2; else if (ch < 0x10000) bytesToWrite = 3; else if (ch < 0x110000) bytesToWrite = 4; else { bytesToWrite = 3; ch = UNI_REPLACEMENT_CHAR; }
        target += bytesToWrite;
        if (target > targetEnd) { source = oldSource; target -= bytesToWrite; result = targetExhausted; break; }
        switch (bytesToWrite) {
            case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6; [[fallthrough]];
            case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6; [[fallthrough]];
            case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6; [[fallthrough]];
            case 1: *--target = (UTF8)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    if (target < targetEnd) *target = '\0'; else if (target == targetEnd) *(target - 1) = '\0';
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
        if (m >= 32) break;
    }
    buf16[m] = 0;
    int result = Utf16_To_Utf8(buf16, transcoding, bufSize - 1, strictConversion);
    if (result != conversionOK) transcoding[0] = '\0';
}

bool WriteBytes(uintptr_t address, const void* data, size_t size) { return WriteIO1(address, data, size); }
bool ReadBytes(uintptr_t address, void* buffer, size_t size) { return KernelDriver::readMemory(target_pid, address, buffer, size); }
bool InitMemoryAccess(pid_t pid) { target_pid = pid; return KernelDriver::isLoaded(); }
void CleanupMemoryAccess() { target_pid = -1; }

#endif // UTILS
