#include <jni.h>
#include <string>
#include <random>
#include <chrono>
#include <cstring>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/memfd.h>
#include <android/log.h>
#include "bklkill_bin.h"

#define TAG "Memfd"

static std::string generateRandomName() {
    static const char alphanum[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string name;
    name.reserve(8);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);
    std::uniform_int_distribution<int> dist(0, sizeof(alphanum) - 2);
    for (int i = 0; i < 8; ++i) {
        name += alphanum[dist(engine)];
    }
    return name;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_microsoft_xboxone_smartglass_Initialize_getDaemonMemFdPath(JNIEnv *env, jclass clazz) {
    std::string daemonName = generateRandomName();
    __android_log_print(ANDROID_LOG_INFO, TAG, "Generated daemon name: %s", daemonName.c_str());

    int fd = syscall(__NR_memfd_create, daemonName.c_str(), MFD_CLOEXEC);
    if (fd < 0) {
        fd = syscall(__NR_memfd_create, daemonName.c_str(), 0);
        if (fd < 0) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "memfd_create failed: %s", strerror(errno));
            return env->NewStringUTF("");
        }
    }

    ssize_t written = write(fd, BKLKILL, BKLKILL_len);
    if (written != (ssize_t)BKLKILL_len) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to write to memfd: %zd vs %u bytes",
                            written, BKLKILL_len);
        close(fd);
        return env->NewStringUTF("");
    }

    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/fd/%d", getpid(), fd);
    __android_log_print(ANDROID_LOG_INFO, TAG, "Daemon path: %s, name: %s", path, daemonName.c_str());

    std::string result = std::string(path) + "|" + daemonName;
    return env->NewStringUTF(result.c_str());
}