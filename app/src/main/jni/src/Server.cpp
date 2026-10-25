#include <jni.h>
#include <unistd.h>
#include <dlfcn.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <android/log.h>

#include "Utils.h"
#include "Offsets.h"
#include "Includes/Dobby.h"
#include "BooleanServer.h" // Ensure this exists in your source

#define LOG_TAG "StormInternal"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Globals
uintptr_t libAddress = 0;
uintptr_t g_currentLocalPlayer = 0;
bool is_injected = false;

// Forward declarations
void CreateDataList(Response& SendResponse);
void ServerThread();

// ==========================================
// THE INTERNAL HOOK (Replaces external write)
// ==========================================
typedef void* (*orig_Update_t)(void* instance, void* method);
orig_Update_t orig_Update = nullptr;

void* hook_Update(void* instance, void* method) {
    // 1. Call the original game function
    void* result = orig_Update(instance, method);

    // 2. Run our cheat logic inside the game loop
    if (!libAddress) {
        libAddress = FindCorrectLibBase();
    }

    // Only run if game is active
    if (libAddress && g_currentLocalPlayer != 0 && Actived.activar) {
        
        // --- AIMBOT (SILENT/HARD) ---
        if (pPlayer.aimhard && g_hasAimHardTarget) {
            std::lock_guard<std::mutex> lock(g_aimHardMutex);
            if (g_aimHardTargetObj != 0 && g_aimHardTargetPos != Vector3::Zero()) {
                
                auto FollowCamera = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->FollowCamera);
                auto Camera = FollowCamera ? Read<uintptr_t>(FollowCamera + currentOffsets->Camera) : 0;
                auto IntPtrCam = Camera ? Read<uintptr_t>(Camera + currentOffsets->intPtrCam) : 0;
                
                if (IntPtrCam) {
                    auto matrix = Read<D3DMatrix>(IntPtrCam + currentOffsets->Matrix);
                    Vector3 cameraPos = GetCameraWorldPosManual(matrix);
                    
                    if (cameraPos != Vector3::Zero()) {
                        Vector3 dir = Vector3::Normalized(g_aimHardTargetPos - cameraPos);
                        Quaternion targetRot = Quaternion::LookRotation(dir, Vector3(0, 1, 0));
                        
                        // DIRECT MEMORY WRITE (No syscalls, no KPM needed for this)
                        Quaternion* aimRot = (Quaternion*)(g_currentLocalPlayer + currentOffsets->AimRotation);
                        aimRot->X = targetRot.X;
                        aimRot->Y = targetRot.Y;
                        aimRot->Z = targetRot.Z;
                        aimRot->W = targetRot.W;
                    }
                }
            }
        }

        // --- SILENT AIM (MAGIC BULLET) ---
        if (pPlayer.silen4a && keepAiming && currentBestTargetObj != 0) {
            uintptr_t aimingInfo = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->AimInfo);
            if (aimingInfo) {
                Vector3 startPos = Read<Vector3>(aimingInfo + currentOffsets->StartPos);
                Vector3 dirToTarget = currentBestTargetPos - startPos;
                float len = sqrtf(dirToTarget.X*dirToTarget.X + dirToTarget.Y*dirToTarget.Y + dirToTarget.Z*dirToTarget.Z);
                if (len > 0.0001f) dirToTarget /= len;
                
                // DIRECT MEMORY WRITE
                Vector3* rayDir = (Vector3*)(aimingInfo + currentOffsets->RayDir);
                rayDir->X = dirToTarget.X;
                rayDir->Y = dirToTarget.Y;
                rayDir->Z = dirToTarget.Z;
            }
        }
    }
    
    // Update the data list for the external UI
    if (is_injected) {
        Response dummy;
        CreateDataList(dummy); 
    }

    return result;
}

// ==========================================
// INITIALIZATION
// ==========================================
__attribute__((constructor))
void on_load() {
    LOGI("StormMods .so Injected into game!");

    // 1. Start the UI Server Thread
    std::thread(ServerThread).detach();

    // 2. Wait for libil2cpp.so to load
    void* handle = dlopen("libil2cpp.so", RTLD_LAZY);
    if (!handle) {
        LOGI("Failed to dlopen libil2cpp.so");
        return;
    }

    // 3. Get the game loop function (Example: il2cpp_thread_attach or a known update offset)
    // Since we don't have the exact Update offset, we hook il2cpp_init just to run our thread safely.
    // The actual cheat loop runs in CreateDataList on a background thread.
    void* target_func = dlsym(handle, "il2cpp_init");
    
    if (target_func) {
        DobbyHook(target_func, (void*)hook_Update, (void**)&orig_Update);
        LOGI("Dobby Hook Installed!");
        is_injected = true;
    }
}

// ==========================================
// UI SERVER THREAD
// ==========================================
void ServerThread() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) return;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(7755); // Port the UI connects to

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    LOGI("Server listening on port 7755");

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
            // Handle toggles from the UI here
            // read(new_socket, buffer, 1024);
        }
    }
}
