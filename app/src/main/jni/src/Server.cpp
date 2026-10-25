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

#define LOG_TAG "StormInternal"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

uintptr_t libAddress = 0;
uintptr_t g_currentLocalPlayer = 0;
bool is_injected = false;

void CreateDataList(Response& SendResponse);
void ServerThread();

// Dobby Hook definitions
typedef void (*orig_Update_t)(void* instance);
orig_Update_t orig_Update = nullptr;

void hook_Update(void* instance) {
    // 1. Call original game function
    orig_Update(instance);

    // 2. Cheat Logic (Runs inside the game, bypassing ACE)
    if (!libAddress) {
        libAddress = FindCorrectLibBase();
    }

    if (libAddress && g_currentLocalPlayer != 0 && Actived.activar) {
        
        // Direct Aimbot Write
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
                        
                        // DIRECT POINTER WRITE
                        Quaternion* aimRot = (Quaternion*)(g_currentLocalPlayer + currentOffsets->AimRotation);
                        aimRot->X = targetRot.X;
                        aimRot->Y = targetRot.Y;
                        aimRot->Z = targetRot.Z;
                        aimRot->W = targetRot.W;
                    }
                }
            }
        }

        // Silent Aim (Magic Bullet) Write
        if (pPlayer.silen4a && keepAiming && currentBestTargetObj != 0) {
            uintptr_t aimingInfo = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->AimInfo);
            if (aimingInfo) {
                Vector3 startPos = Read<Vector3>(aimingInfo + currentOffsets->StartPos);
                Vector3 dirToTarget = currentBestTargetPos - startPos;
                float len = sqrtf(dirToTarget.X*dirToTarget.X + dirToTarget.Y*dirToTarget.Y + dirToTarget.Z*dirToTarget.Z);
                if (len > 0.0001f) dirToTarget /= len;
                
                Vector3* rayDir = (Vector3*)(aimingInfo + currentOffsets->RayDir);
                rayDir->X = dirToTarget.X;
                rayDir->Y = dirToTarget.Y;
                rayDir->Z = dirToTarget.Z;
            }
        }
    }
    
    // Update ESP data
    if (is_injected) {
        Response dummy;
        CreateDataList(dummy); 
    }
}

__attribute__((constructor))
void on_load() {
    LOGI("BKLKILL Payload Injected!");

    // 1. Start external UI server
    std::thread(ServerThread).detach();

    // 2. Hook the game's init function
    void* handle = dlopen("libil2cpp.so", RTLD_LAZY);
    if (handle) {
        // You need to find the exact offset of a function that runs every frame.
        // il2cpp_init is usually just a one-time init. 
        // If the aimbot doesn't work, we will need to find the actual Update offset from dump.cs.
        void* target_func = dlsym(handle, "il2cpp_init");
        if (target_func) {
            DobbyHook(target_func, (void*)hook_Update, (void**)&orig_Update);
            LOGI("Dobby Hook Installed on il2cpp_init");
            is_injected = true;
        }
    }
}

void ServerThread() {
    // Your existing UI socket code here
}
