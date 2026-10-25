#include <dirent.h>
#include "Server.h"
#include "Includes/Encrypt/oxorany_include.h"
#include <android/log.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <atomic>
#include <thread>
#include <stdarg.h>
#include <fstream>
#include <unordered_map>
#include <random>
#include <cmath>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unordered_set>
#include "Namegun.h"
#include "BooleanServer.h"
#include <sys/mman.h>
#include <fstream>
#include <mutex>
#include <unordered_set>
#include <sys/stat.h>
#include <android/log.h>

#define NG_LOG_TAG "WeaponScanner"
#define NG_LOGI(...) __android_log_print(ANDROID_LOG_INFO, NG_LOG_TAG, __VA_ARGS__)

// Lista de ID-uri deja cunoscute (mirror al Namegun::Init)
static const std::unordered_set<int> kKnownWeaponIds = {
        // SPECIALS
        6016, 10006, 2102, 9476, 9479, 21001, 7812,
        // BOTS
        0,
        // ASSAULT RIFLES
        2,80,81,82,7821,6,7802,11,63,126,127,12,178,179,180,7803,
        14,629,70,7859,24,67,130,131,635,28,33,7805,39,7806,
        46,193,194,195,7807,47,7808,57,7809,73,74,7810,7811,
        // MARKSMAN
        18,7817,26,72,48,7818,89,7820,21006,21007,21008,21009,7801,
        // MACHINE GUNS
        19,631,71,7866,30,61,122,123,7813,54,7814,
        // SMGs
        7,7840,8,60,120,121,7841,13,62,124,125,7842,15,7843,
        32,7844,35,43,637,21020,7863,49,69,88,228,229,230,7847,150,7848,
        // SHOTGUNS
        5,184,185,186,7835,29,41,7862,119,50,7837,86,7855,181,182,21002,7839,
        // SNIPERS
        4,65,7858,21,64,128,129,7825,45,7822,75,805,806,807,7823,78,197,7824,
        // PISTOLS
        3,56,9,10,20,25,55,58,93,
        // MELEE
        16,17,27,34,51,53,
        // FIST
        1,
        // GRENADES
        601,603,1201,1204,602,608,
        // OTHERS
        23,36,196,
        // MISC
        100,99,617,1401,1006,1015
};

static std::unordered_set<int> g_loggedUnknownWeapons;
static std::mutex g_weaponLogMutex;

static const char* kWeaponLogPaths[] = {
        "/data/media/0/Download/unknown_weapon_ids.txt",
        "/sdcard/Download/unknown_weapon_ids.txt",
        "/storage/emulated/0/Download/unknown_weapon_ids.txt",
        "/data/local/tmp/unknown_weapon_ids.txt",
};

static void CheckAndLogWeapon(short gunId) {
    if (gunId == 0) return;
    int adjustedId = gunId < 0 ? gunId + 25000 : gunId;
    bool known = kKnownWeaponIds.count(adjustedId) > 0;
    NG_LOGI("[Weapon] raw=%d adjusted=%d status=%s", (int)gunId, adjustedId, known ? "KNOWN" : "UNKNOWN");
    if (known) return;

    std::lock_guard<std::mutex> lock(g_weaponLogMutex);
    if (g_loggedUnknownWeapons.count(adjustedId)) return;
    g_loggedUnknownWeapons.insert(adjustedId);

    mkdir("/data/media/0/Download", 0775);
    mkdir("/data/media/0", 0775);
    mkdir("/data/local/tmp", 0777);

    for (const char* path : kWeaponLogPaths) {
        std::ofstream f(path, std::ios::app);
        if (f.is_open()) {
            f << adjustedId << "\n"; f.flush(); f.close(); chmod(path, 0664);
            NG_LOGI("[Weapon] SAVED unknown ID %d -> %s", adjustedId, path);
            return;
        }
    }
}

const Offsets* currentOffsets = &FFMax;
void setCurrentVersion(const std::string& version) {
    if (version == "v2.133.x") currentOffsets = &FFMaxInd;
    else currentOffsets = &FFMax;
}

std::random_device rd;
uintptr_t currentBestTargetObj = 0;
std::atomic<bool> keepAiming{false};
Vector3 currentBestTargetPos = Vector3::Zero();
Vector3 finalSilentPos = Vector3::Zero();
Vector3 finalAimCollider = Vector3::Zero();

std::unordered_map<uintptr_t, bool> g_isScaled;
static bool g_isDiveFrozen = false;
static bool diveact = false;
Vector3 g_originalDivePos = Vector3::Zero();
float g_diveDownDistance = 0.9f;
uintptr_t g_currentLocalPlayer = 0;
uintptr_t g_pullEnemyTargetAddr = 0;
std::atomic<bool> g_hasPullTargetV2{false};
Vector3 g_aimHardTargetPos = Vector3::Zero();
uintptr_t g_aimHardTargetObj = 0;
std::atomic<bool> g_hasAimHardTarget{false};
std::mutex g_aimHardMutex;
uintptr_t g_aimbotTargetObj = 0;
Vector3 g_aimbotTargetPos = Vector3::Zero();
uintptr_t g_aimLagTargetObj = 0;
Vector3 g_aimLagTargetPos = Vector3::Zero();
std::atomic<bool> g_hasAimLagTarget{false};
bool aimHead = false;
float currentAngle = 0.0f;
float jitterTime = 0.0f;
float spinSpeed = 0.0f;
int spinDirection = 1;
float changeTimer = 0.0f;

static const std::unordered_set<short> blockedWeaponIds = {6016, 10006, -15524, -15521, 601, 603, 1201, 1204, 602, 608, 23, 1401, 1006, 1015 };

bool IsGameActive() {
    if (libAddress == 0) return false;
    uintptr_t gameFacade = Read<uintptr_t>(libAddress + currentOffsets->GameFacade_c);
    if (!gameFacade) return false;
    uintptr_t staticClass = Read<uintptr_t>(gameFacade + currentOffsets->StaticClass);
    if (!staticClass) return false;
    uintptr_t m_Match = Read<uintptr_t>(staticClass + currentOffsets->m_Match);
    if (!m_Match) return false;
    int matchState = Read<int>(m_Match + currentOffsets->MatchRun);
    return (matchState == 1); // 1 means In Match
}

Vector3 GetCameraWorldPosManual(D3DMatrix m) {
    float a11 = m._11, a12 = m._21, a13 = m._31, a21 = m._12, a22 = m._22, a23 = m._32, a31 = m._14, a32 = m._24, a33 = m._34, b1 = -m._41, b2 = -m._42, b3 = -m._44;
    float det = a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31);
    if (fabs(det) < 0.0001f) return Vector3::Zero();
    float inv = 1.0f / det;
    return Vector3(
            (b1 * (a22 * a33 - a23 * a32) - a12 * (b2 * a33 - a23 * b3) + a13 * (b2 * a32 - a22 * b3)) * inv,
            (a11 * (b2 * a33 - a23 * b3) - b1 * (a21 * a33 - a23 * a31) + a13 * (a21 * b3 - b2 * a31)) * inv,
            (a11 * (a22 * b3 - b2 * a32) - a12 * (a21 * b3 - b2 * a31) + b1 * (a21 * a32 - a22 * a31)) * inv
    );
}

void getUTF8(char* dst, uintptr_t addr) {
    if (addr < 0x10000000) { dst[0] = '\0'; return; }
    int stringLen = Read<int>(addr + 0x10);
    if (stringLen <= 0 || stringLen > 64) stringLen = 32;
    unsigned char raw[128];
    for (int i = 0; i < (stringLen * 2); i++) raw[i] = Read<unsigned char>(addr + 0x14 + i);
    int j = 0;
    for (int i = 0; i < (stringLen * 2); i += 2) {
        unsigned short unicode = raw[i] | (raw[i + 1] << 8);
        if (unicode == 0) break;
        if (unicode < 0x80) dst[j++] = (char)unicode;
        else if (unicode < 0x800) { dst[j++] = (char)((unicode >> 6) | 0xC0); dst[j++] = (char)((unicode & 0x3F) | 0x80); }
        else if (unicode >= 0xD800 && unicode <= 0xDBFF) {
            i += 2; unsigned short low = raw[i] | (raw[i + 1] << 8);
            unsigned int codepoint = 0x10000 + ((unicode - 0xD800) << 10) + (low - 0xDC00);
            dst[j++] = (char)((codepoint >> 18) | 0xF0); dst[j++] = (char)(((codepoint >> 12) & 0x3F) | 0x80);
            dst[j++] = (char)(((codepoint >> 6) & 0x3F) | 0x80); dst[j++] = (char)((codepoint & 0x3F) | 0x80);
        } else { dst[j++] = (char)((unicode >> 12) | 0xE0); dst[j++] = (char)(((unicode >> 6) & 0x3F) | 0x80); dst[j++] = (char)((unicode & 0x3F) | 0x80); }
        if (j >= 60) break;
    } dst[j] = '\0';
}

bool is_elf_header(uintptr_t addr) {
    if (addr == 0) return false;
    unsigned char magic[4]; magic[0] = Read<uint8_t>(addr); magic[1] = Read<uint8_t>(addr + 1); magic[2] = Read<uint8_t>(addr + 2); magic[3] = Read<uint8_t>(addr + 3);
    return (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
}

uintptr_t FindCorrectLibBase() {
    char mapsPath[64]; snprintf(mapsPath, sizeof(mapsPath), "/proc/%d/maps", target_pid);
    FILE* maps = fopen(mapsPath, "r"); if (!maps) return 0;
    char line[512]; uintptr_t elf1 = 0, elf2 = 0, elf3 = 0;
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, "libil2cpp.so")) {
            uintptr_t start = 0; sscanf(line, "%lx", &start);
            if (is_elf_header(start)) {
                if (elf1 == 0) elf1 = start; else if (elf2 == 0) elf2 = start; else if (elf3 == 0) elf3 = start;
            }
        }
    }
    fclose(maps); uintptr_t testOffsets[] = {elf1, elf2, elf3};
    for (int i = 0; i < 3; i++) { if (testOffsets[i] == 0) continue; uintptr_t gameFacade = Read<uintptr_t>(testOffsets[i] + currentOffsets->GameFacade_c); if (gameFacade > 0x100000000) return testOffsets[i]; }
    return elf2;
}

uintptr_t GetEnemyPosAddr(uintptr_t enemyObj) {
    if (!enemyObj) return 0; uintptr_t headTF = GetPlayerPeTF(enemyObj); if (!headTF) return 0;
    uintptr_t v1 = Read<uintptr_t>(headTF + currentOffsets->Transform); uintptr_t v2 = Read<uintptr_t>(v1 + currentOffsets->Transform2);
    uintptr_t v3 = Read<uintptr_t>(v2 + currentOffsets->MatrixPtr); return v3 + currentOffsets->Position;
}

uintptr_t GetRootPosAddr(uintptr_t player) {
    if (!player) return 0; uintptr_t rootTF = GetPlayerHipTF(player); if (!rootTF) return 0;
    uintptr_t v1 = Read<uintptr_t>(rootTF + currentOffsets->Transform); uintptr_t v2 = Read<uintptr_t>(v1 + currentOffsets->Transform2);
    uintptr_t matrixPtr = Read<uintptr_t>(v2 + currentOffsets->MatrixPtr); return matrixPtr + currentOffsets->Position;
}

void FlyHackTh() {
    static float g_flyGroundY = 0.0f; static bool g_flyInit = false;
    while (true) {
        if (pPlayer.FlyHack && g_currentLocalPlayer != 0 && IsGameActive()) {
            uintptr_t rootPosAddr = GetRootPosAddr(g_currentLocalPlayer);
            if (rootPosAddr != 0) {
                Vector3 currentPos = Read<Vector3>(rootPosAddr);
                if (!g_flyInit) { g_flyGroundY = currentPos.Y; g_flyInit = true; }
                currentPos.Y = g_flyGroundY + 4.5f; Write<Vector3>(rootPosAddr, currentPos); Write<bool>(g_currentLocalPlayer + 0x2098, true);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        } else { if (g_flyInit) { g_flyInit = false; if (g_currentLocalPlayer != 0) Write<bool>(g_currentLocalPlayer + 0x2098, false); } }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

void SetEnemyScale(uintptr_t enemyObj, bool scaleDown) {
    if (!enemyObj) return; bool isCurrentlyScaled = g_isScaled[enemyObj]; if (scaleDown == isCurrentlyScaled) return;
    uintptr_t rootTF = GetPlayerPeTF(enemyObj); if (!rootTF) return;
    uintptr_t v1 = Read<uintptr_t>(rootTF + currentOffsets->Transform); if (!v1) return;
    uintptr_t v2 = Read<uintptr_t>(v1 + currentOffsets->Transform2); if (!v2) return;
    uintptr_t matrix = Read<uintptr_t>(v2 + currentOffsets->MatrixPtr); if (!matrix) return;
    if (scaleDown) { Vector3 targetScale = {-1.3f, -1.3f, -1.3f}; Write<Vector3>(matrix + currentOffsets->Scale, targetScale); g_isScaled[enemyObj] = true; } 
    else { Vector3 normalScale = {1.0f, 1.0f, 1.0f}; Write<Vector3>(matrix + currentOffsets->Scale, normalScale); g_isScaled[enemyObj] = false; }
}

void DiveKillThread() {
    static uintptr_t lastLocalPlayer = 0;
    while (true) {
        uintptr_t currentLocal = g_currentLocalPlayer;
        if (currentLocal != lastLocalPlayer) { g_isDiveFrozen = false; g_originalDivePos = Vector3::Zero(); lastLocalPlayer = currentLocal; }
        if (pPlayer.DiveKill && currentLocal != 0 && IsGameActive()) {
            uintptr_t rootAddr = GetEnemyPosAddr(currentLocal);
            if (rootAddr) {
                if (!g_isDiveFrozen) { g_originalDivePos = Read<Vector3>(rootAddr); if (g_originalDivePos.Y != 0) g_isDiveFrozen = true; }
                if (g_isDiveFrozen) { Vector3 targetPos = g_originalDivePos; targetPos.Y -= g_diveDownDistance; Write<Vector3>(rootAddr, targetPos); diveact = true; }
            }
        } else if (diveact) {
            if (g_isDiveFrozen && currentLocal != 0) {
                uintptr_t rootAddr = GetEnemyPosAddr(currentLocal);
                if (rootAddr) { Vector3 currentPos = Read<Vector3>(rootAddr); currentPos.Y += g_diveDownDistance; Write<Vector3>(rootAddr, currentPos); diveact = false; }
                g_isDiveFrozen = false; g_originalDivePos = Vector3::Zero();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
}

bool isWithinFOV(Vector3 screenPos, float fovRadius) {
    float screenCenterX = g_screenWidth / 2.0f; float screenCenterY = g_screenHeight / 2.0f;
    float deltaX = screenPos.X - screenCenterX; float deltaY = screenPos.Y - screenCenterY;
    return sqrt(deltaX * deltaX + deltaY * deltaY) <= fovRadius;
}

bool isWithinPullFOV(Vector3 screenPos, float pullFovRadius) {
    float screenCenterX = g_screenWidth / 2.0f; float screenCenterY = g_screenHeight / 2.0f;
    float deltaX = screenPos.X - screenCenterX; float deltaY = screenPos.Y - screenCenterY;
    return sqrt(deltaX * deltaX + deltaY * deltaY) <= pullFovRadius;
}

// --- CRASH FIX APPLIED HERE (DEEP SLEEP) ---
void SilentAimTh() {
    while (true) {
        if (!pPlayer.silen4a || g_currentLocalPlayer == 0 || !IsGameActive()) {
            keepAiming = false;
            currentBestTargetObj = 0;
            currentBestTargetPos = Vector3::Zero();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Sleep 1 full second
            continue;
        }

        bool isFiring = Read<bool>(g_currentLocalPlayer + currentOffsets->isFiring);
        if (isFiring) {
            if (keepAiming && currentBestTargetObj != 0) {
                uintptr_t aimingInfo = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->AimInfo);
                if (aimingInfo) {
                    Vector3 startPos = Read<Vector3>(aimingInfo + currentOffsets->StartPos);
                    Vector3 dirToTarget = currentBestTargetPos - startPos;
                    float len = sqrtf(dirToTarget.X*dirToTarget.X + dirToTarget.Y*dirToTarget.Y + dirToTarget.Z*dirToTarget.Z);
                    if (len > 0.0001f) dirToTarget /= len; else dirToTarget = Vector3(0, 0, 1);
                    if (startPos.X != 0 && startPos.Y != 0 && startPos.Z != 0) {
                        Write<Vector3>(aimingInfo + currentOffsets->RayDir, dirToTarget);
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void RotateTh() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    while (true) {
        if ((pPlayer.SpinBot || pPlayer.WallBug) && g_currentLocalPlayer != 0 && IsGameActive()) {
            uintptr_t rootPtr = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->RootBone);
            if (rootPtr) {
                uintptr_t t1 = Read<uintptr_t>(rootPtr + currentOffsets->Transform); uintptr_t t2 = t1 ? Read<uintptr_t>(t1 + currentOffsets->Transform2) : 0;
                uintptr_t matrixPtr = t2 ? Read<uintptr_t>(t2 + currentOffsets->MatrixPtr) : 0;
                if (matrixPtr) {
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
                    lastTime = currentTime;
                    if (pPlayer.SpinBot) {
                        changeTimer += deltaTime;
                        if (changeTimer > 1.2f) { changeTimer = 0.f; spinSpeed = (float)(30 + (rand() % 71)) / 10.0f; spinDirection = (rand() % 2 == 0) ? 1 : -1; }
                        currentAngle += (spinSpeed * spinDirection) * (deltaTime * 60.0f);
                        if (currentAngle > 360.f) currentAngle -= 360.f; if (currentAngle < 0.f) currentAngle += 360.f;
                        float rad = currentAngle * (3.14159265f / 180.0f); float halfRad = rad * 0.5f;
                        Quaternion newRot; newRot.X = 0.f; newRot.Z = 0.f; newRot.Y = sinf(halfRad); newRot.W = cosf(halfRad);
                        Write<Quaternion>(matrixPtr + currentOffsets->Rotation, newRot);
                    }
                    if (pPlayer.WallBug) {
                        jitterTime += deltaTime * 8.0f;
                        uintptr_t followCamera = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->FollowCamera); uintptr_t camera = followCamera ? Read<uintptr_t>(followCamera + currentOffsets->Camera) : 0;
                        uintptr_t intPtrCam = camera ? Read<uintptr_t>(camera + currentOffsets->intPtrCam) : 0;
                        if (intPtrCam) {
                            D3DMatrix matrix = Read<D3DMatrix>(intPtrCam + currentOffsets->Matrix);
                            Vector3 forward(-matrix._31, -matrix._32, matrix._33); float mag = sqrtf(forward.X * forward.X + forward.Y * forward.Y + forward.Z * forward.Z);
                            if (mag > 0.0001f) { forward.X /= mag; forward.Y /= mag; forward.Z /= mag; }
                            float angleDeg = 75.0f + sinf(jitterTime) * 4.0f; float rad = angleDeg * (3.14159265f / 180.0f); float half = rad * 0.5f;
                            float s = sinf(half); float c = cosf(half); float tiltX = -forward.Z; float tiltZ = forward.X;
                            float len = sqrtf(tiltX * tiltX + tiltZ * tiltZ); if (len > 0.001f) { tiltX /= len; tiltZ /= len; }
                            Quaternion q; q.X = tiltX * s; q.Y = 0.0f; q.Z = tiltZ * s; q.W = c;
                            Write<Quaternion>(matrixPtr + currentOffsets->Rotation, q);
                        }
                    }
                }
            }
        } else { lastTime = std::chrono::high_resolution_clock::now(); }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void TeleportToMarkThread() {
    while (true) {
        if (pPlayer.TPMark && g_currentLocalPlayer != 0 && IsGameActive()) {
            uintptr_t static_fields_addr = Read<uintptr_t>(libAddress + currentOffsets->GameFacade_c);
            if (static_fields_addr != 0) {
                uintptr_t static_fields = Read<uintptr_t>(static_fields_addr + currentOffsets->StaticClass);
                if (static_fields != 0) {
                    uintptr_t currentGame = Read<uintptr_t>(static_fields + 0x0);
                    if (currentGame != 0) {
                        uintptr_t uiInGameScene = Read<uintptr_t>(currentGame + currentOffsets->uiInGameScene);
                        if (uiInGameScene != 0) {
                            uintptr_t bigMapCtrl = Read<uintptr_t>(uiInGameScene + currentOffsets->bigMapCtrl);
                            if (bigMapCtrl != 0) {
                                uintptr_t mapContentCtrl = Read<uintptr_t>(bigMapCtrl + currentOffsets->mapContentCtrl);
                                if (mapContentCtrl != 0) {
                                    uintptr_t localMapMarkController = Read<uintptr_t>(mapContentCtrl + currentOffsets->localMapMarkController);
                                    if (localMapMarkController != 0) {
                                        Vector3 targetPosition = Read<Vector3>(localMapMarkController + currentOffsets->markPosition);
                                        if (targetPosition != Vector3::Zero()) {
                                            uintptr_t rootPtr = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->RootBone);
                                            if (rootPtr != 0) {
                                                uintptr_t t1 = Read<uintptr_t>(rootPtr + currentOffsets->Transform);
                                                if (t1 != 0) {
                                                    uintptr_t t2 = Read<uintptr_t>(t1 + currentOffsets->Transform2);
                                                    if (t2 != 0) {
                                                        uintptr_t matrixPtr = Read<uintptr_t>(t2 + currentOffsets->MatrixPtr);
                                                        if (matrixPtr != 0) Write<Vector3>(matrixPtr + currentOffsets->Position, targetPosition);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void AimColliderTh() {
    while (true) {
        if (pPlayer.aimbotlock && g_currentLocalPlayer != 0 && IsGameActive()) {
            bool IsFiring = Read<bool>(g_currentLocalPlayer + currentOffsets->isFiring);
            if (IsFiring && g_aimbotTargetObj != 0 && g_aimbotTargetPos != Vector3::Zero()) {
                auto FollowCamera = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->FollowCamera);
                auto Camera = FollowCamera ? Read<uintptr_t>(FollowCamera + currentOffsets->Camera) : 0;
                auto IntPtrCam = Camera ? Read<uintptr_t>(Camera + currentOffsets->intPtrCam) : 0;
                if (IntPtrCam) {
                    auto matrix = Read<D3DMatrix>(IntPtrCam + currentOffsets->Matrix);
                    Vector3 screenPos = WorldToScreenPoint(matrix, g_aimbotTargetPos);
                    if (screenPos.X >= 0 && screenPos.X <= g_screenWidth && screenPos.Y >= 0 && screenPos.Y <= g_screenHeight) {
                        auto HeadCollider = Read<uintptr_t>(g_aimbotTargetObj + currentOffsets->ColliderHead);
                        if (HeadCollider != 0) Write<uintptr_t>(g_aimbotTargetObj + currentOffsets->ColliderBody, HeadCollider);
                        int randomDelay = 20 + (rand() % 10);
                        std::this_thread::sleep_for(std::chrono::milliseconds(randomDelay));
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void PullEnemyTh() {
    static uintptr_t lockedEnemy = 0; static float depthLockValue = 0.0f; static Vector3 currentBoneOffset = Vector3::Zero();
    static Vector3 smoothedOffset = Vector3::Zero(); static float lastSwitchTime = 0.0f; static bool wasFiringLastFrame = false;
    static Vector3 originalRootPos = Vector3::Zero(); static float targetDepthOffset = 0.0f; static float currentDepthOffset = 0.0f;
    const float MIN_FORWARD_DIST = 0.7f;

    while (true) {
        if (pPlayer.pullenemy && g_currentLocalPlayer != 0 && IsGameActive()) {
            uintptr_t enemyObj = g_pullEnemyTargetAddr;
            bool isFiring = Read<bool>(g_currentLocalPlayer + currentOffsets->isFiring);
            auto Weapon = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->Weapon);
            bool isSighted = false;
            if (Weapon) { uintptr_t sightPtr = Read<uintptr_t>(Weapon + currentOffsets->isSight); isSighted = (sightPtr != 0); }
            bool active = (isFiring || isSighted);

            if (!active) { lockedEnemy = 0; wasFiringLastFrame = false; std::this_thread::sleep_for(std::chrono::milliseconds(10)); continue; }

            if (enemyObj != 0) {
                auto headTF = GetPlayerHeadTF(enemyObj); auto rootTF = GetPlayerPeTF(enemyObj);
                if (headTF && rootTF) {
                    Vector3 headPos = GetNodePosition(headTF); Vector3 rootPos = GetNodePosition(rootTF);
                    Vector3 chosenBone = headPos; currentBoneOffset = chosenBone - rootPos;

                    if (lockedEnemy != enemyObj) {
                        smoothedOffset = currentBoneOffset; originalRootPos = rootPos;
                        auto followCamera = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->FollowCamera); auto camera = followCamera ? Read<uintptr_t>(followCamera + currentOffsets->Camera) : 0;
                        auto intPtrCam = camera ? Read<uintptr_t>(camera + currentOffsets->intPtrCam) : 0;
                        if (intPtrCam) { D3DMatrix matrix = Read<D3DMatrix>(intPtrCam + currentOffsets->Matrix); Vector3 camPos = GetCameraWorldPosManual(matrix); depthLockValue = Vector3::Distance(camPos, chosenBone); }
                        lockedEnemy = enemyObj; wasFiringLastFrame = true; targetDepthOffset = 0.0f; currentDepthOffset = 0.0f;
                    }

                    smoothedOffset = Vector3::Lerp(smoothedOffset, currentBoneOffset, 0.05f);
                    const float PULL_STRENGTH = 5.0f; const float SMOOTH_SPEED = 0.05f;
                    targetDepthOffset = active ? -PULL_STRENGTH : 0.0f;
                    currentDepthOffset += (targetDepthOffset - currentDepthOffset) * SMOOTH_SPEED;
                    if (currentDepthOffset > 0.0f) currentDepthOffset = 0.0f;
                    if (currentDepthOffset < -PULL_STRENGTH) currentDepthOffset = -PULL_STRENGTH;

                    auto followCamera = Read<uintptr_t>(g_currentLocalPlayer + currentOffsets->FollowCamera); auto camera = followCamera ? Read<uintptr_t>(followCamera + currentOffsets->Camera) : 0;
                    auto intPtrCam = camera ? Read<uintptr_t>(camera + currentOffsets->intPtrCam) : 0;

                    if (intPtrCam) {
                        D3DMatrix matrix = Read<D3DMatrix>(intPtrCam + currentOffsets->Matrix); Vector3 camPos = GetCameraWorldPosManual(matrix);
                        Vector3 viewDir(matrix._13, matrix._23, matrix._33);
                        float mag = sqrtf(viewDir.X * viewDir.X + viewDir.Y * viewDir.Y + viewDir.Z * viewDir.Z);
                        if (mag > 0.0001f) { viewDir.X /= mag; viewDir.Y /= mag; viewDir.Z /= mag; }
                        Vector3 horizDir(viewDir.X, 0.0f, viewDir.Z);
                        float horizMag = sqrtf(horizDir.X * horizDir.X + horizDir.Z * horizDir.Z);
                        if (horizMag > 0.0001f) { horizDir.X /= horizMag; horizDir.Z /= horizMag; } else { horizDir = Vector3(0, 0, 1); }

                        float adjustedDepth = depthLockValue + currentDepthOffset; if (adjustedDepth < 0.1f) adjustedDepth = 0.1f;
                        Vector3 desiredPoint = camPos + (viewDir * adjustedDepth); Vector3 finalRootPos = desiredPoint - smoothedOffset;

                        const float MAX_DELTA_X = 5.3f; const float MAX_DELTA_Z = 5.3f;
                        float deltaX = finalRootPos.X - originalRootPos.X; float deltaZ = finalRootPos.Z - originalRootPos.Z;
                        if (deltaX > MAX_DELTA_X) deltaX = MAX_DELTA_X; if (deltaX < -MAX_DELTA_X) deltaX = -MAX_DELTA_X;
                        if (deltaZ > MAX_DELTA_Z) deltaZ = MAX_DELTA_Z; if (deltaZ < -MAX_DELTA_Z) deltaZ = -MAX_DELTA_Z;
                        finalRootPos.X = originalRootPos.X + deltaX; finalRootPos.Z = originalRootPos.Z + deltaZ; finalRootPos.Y = originalRootPos.Y;

                        auto localRootTF = GetPlayerPeTF(g_currentLocalPlayer);
                        if (localRootTF) {
                            Vector3 playerPos = GetNodePosition(localRootTF); Vector3 toEnemy = finalRootPos - playerPos;
                            float forwardDist = Vector3::Dot(toEnemy, horizDir);
                            if (forwardDist < MIN_FORWARD_DIST) { finalRootPos.X += horizDir.X * (MIN_FORWARD_DIST - forwardDist); finalRootPos.Z += horizDir.Z * (MIN_FORWARD_DIST - forwardDist); }
                        }
                        uintptr_t rootTF_enemy = GetPlayerPeTF(enemyObj);
                        if (rootTF_enemy) {
                            uintptr_t t1 = Read<uintptr_t>(rootTF_enemy + currentOffsets->Transform); uintptr_t t2 = t1 ? Read<uintptr_t>(t1 + currentOffsets->Transform2) : 0;
                            uintptr_t matrixPtr = t2 ? Read<uintptr_t>(t2 + currentOffsets->MatrixPtr) : 0;
                            if (matrixPtr) Write<Vector3>(matrixPtr + currentOffsets->Position, finalRootPos);
                        }
                    }
                }
            } else { lockedEnemy = 0; wasFiringLastFrame = false; }
        } else { lockedEnemy = 0; wasFiringLastFrame = false; }
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
    }
}

// --- CRASH FIX APPLIED HERE (DEEP SLEEP) ---
void AimHardTh() {
    static std::random_device rd; 
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> sleepDist(40, 100);

    while (true) {
        if (!pPlayer.aimhard || g_currentLocalPlayer == 0 || !IsGameActive()) {
            std::lock_guard<std::mutex> lock(g_aimHardMutex);
            g_hasAimHardTarget = false; 
            g_aimHardTargetObj = 0; 
            g_aimHardTargetPos = Vector3::Zero();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Sleep 1 full second
            continue;
        }

        bool isFiring = Read<bool>(g_currentLocalPlayer + currentOffsets->isFiring);
        if (isFiring) {
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
                        Write<Quaternion>(g_currentLocalPlayer + currentOffsets->AimRotation, targetRot);
                        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

bool ReadWeaponOnHand(uintptr_t player, uintptr_t &weapon) {
    weapon = 0; uintptr_t inventoryManager = Read<uintptr_t>(player + currentOffsets->inventoryManager); if (!inventoryManager) return false;
    bool isCombined = Read<bool>(inventoryManager + currentOffsets->isCombined);
    if (isCombined) weapon = Read<uintptr_t>(inventoryManager + currentOffsets->combinedWeapon);
    else weapon = Read<uintptr_t>(inventoryManager + currentOffsets->itemOnHand);
    return weapon != 0;
}

uint64_t ReadWeaponShortId(uintptr_t weapon) {
    uintptr_t itemData = Read<uintptr_t>(weapon + currentOffsets->itemData); if (!itemData) return 0;
    return Read<uint64_t>(itemData + currentOffsets->shortId);
}

uint64_t ReadCurrentWeapon(uintptr_t player) {
    uintptr_t weapon = 0; if (ReadWeaponOnHand(player, weapon)) return ReadWeaponShortId(weapon); return 0;
}

static Response lastValidResponse = {};
static int framesWithoutPlayers = 0;
static const int MAX_FRAMES_WITHOUT_PLAYERS = 5;

void CreateDataList(Response& SendResponse) {
    SendResponse.EnemiesInFront = 0; SendResponse.EnemiesBehind = 0;

    if (Actived.activar) {
        float bestDistance = 999.0f; bool hasValidTarget = false;
        Vector3 bestTargetPos = Vector3::Zero(); uintptr_t bestTargetObj = 0;
        Vector3 finalAimHardBaseHead = Vector3::Zero(); Vector3 finalAimHardBaseHip = Vector3::Zero();
        SendResponse.PlayerCount = 0;
        
        float bestPullCrosshairDist = 999.0f; float screenCenterX = g_screenWidth / 2.0f; float screenCenterY = g_screenHeight / 2.0f;
        uintptr_t bestPullTargetObj = 0; bool hasPullTarget = false;
        static bool SpeedAct = false; static bool GhostAct = false; static float OriSpeed = 0.0f; static bool originalSpeedRead = false;
        static int silentCounter = 0; static int lastAmmo = -1; static uintptr_t lastWeapon = 0;
        static int lastHeadRate = -1, lastChestRate = -1; static std::vector<bool> pattern; static int totalPattern = 0;
        float bestAimLagDist = 300.0f; uintptr_t bestAimLagObj = 0; Vector3 bestAimLagPos = Vector3::Zero();
        static bool oriPc = false;

        int rawHead = (int)pPlayer.HeadRate; int rawChest = (int)pPlayer.ChestRate;
        int headRate = (rawHead > 3) ? 3 : rawHead; int chestRate = (rawChest > 6) ? 6 : rawChest;
        if (headRate == 0 && chestRate == 0) headRate = 1;
        int total = headRate + chestRate;

        if (headRate != lastHeadRate || chestRate != lastChestRate) {
            lastHeadRate = headRate; lastChestRate = chestRate; totalPattern = total;
            pattern.clear(); pattern.resize(total, false);
            if (headRate > 0 && total > 0) {
                float step = (float)total / headRate;
                for (int i = 0; i < headRate; ++i) {
                    int pos = (int)(i * step + 0.5f); if (pos >= total) pos = total - 1; pattern[pos] = true;
                }
            }
        }

        uintptr_t GameFacade_c = Read<uintptr_t>(libAddress + currentOffsets->GameFacade_c);
        if (GameFacade_c) {
            uintptr_t P2 = Read<uintptr_t>(GameFacade_c + currentOffsets->StaticClass);
            if(P2) {
                uintptr_t BaseGame = Read<uintptr_t>(P2);
                if (BaseGame) {
                    uintptr_t m_Match = Read<uintptr_t>(BaseGame + currentOffsets->m_Match);
                    if (m_Match) {
                        auto MatchIsRunning = Read<int>(m_Match + currentOffsets->MatchRun);
                        if(MatchIsRunning == 1) {
                            uintptr_t livePlayer = Read<uintptr_t>(m_Match + currentOffsets->localPlayer);
                            static uintptr_t savedLocalPlayer = 0;
                            if (livePlayer != 0) savedLocalPlayer = livePlayer;
                            uintptr_t localPlayer = (livePlayer != 0) ? livePlayer : savedLocalPlayer;
                            g_currentLocalPlayer = livePlayer;
                            uint64_t localWeaponId = ReadCurrentWeapon(livePlayer);
                            CheckAndLogWeapon((short)localWeaponId);
                            bool isLocalWeaponBlocked = blockedWeaponIds.find((short)localWeaponId) != blockedWeaponIds.end();
                            
                            if (localPlayer != 0) {
                                uintptr_t FollowCamera = Read<uintptr_t>(localPlayer + currentOffsets->FollowCamera);
                                if (FollowCamera != 0) {
                                    uintptr_t Camera = Read<uintptr_t>(FollowCamera + currentOffsets->Camera);
                                    if (Camera != 0) {
                                        uintptr_t IntPtrCam = Read<uintptr_t>(Camera + currentOffsets->intPtrCam);
                                        if (IntPtrCam != 0) {
                                            auto matrix = Read<D3DMatrix>(IntPtrCam + currentOffsets->Matrix);
                                            auto dictionary = Read<MonoDictionary *>(BaseGame + currentOffsets->dictionary);

                                            if (dictionary) {
                                                int numValues = dictionary->getNumValues(); uintptr_t valuesPtr = dictionary->getValues();
                                                Vector3 camPos = GetCameraWorldPosManual(matrix);
                                                Vector3 LocalPosition = (livePlayer != 0) ? GetNodePosition(GetPlayerHeadTF(livePlayer)) : camPos;
                                                if (camPos == Vector3::Zero()) camPos = LocalPosition;

                                                uintptr_t LocalAtri = Read<uintptr_t>(localPlayer + currentOffsets->Attributes);
                                                uintptr_t DamageContext = Read<uintptr_t>(LocalAtri + currentOffsets->DamageCont);
                                                uintptr_t weapon = Read<uintptr_t>(DamageContext + currentOffsets->Weaponv1);
                                                if (weapon) {
                                                    if (weapon != lastWeapon) { silentCounter = 0; lastAmmo = -1; lastWeapon = weapon; }
                                                    int currentAmmo = Read<int>(weapon + currentOffsets->AmmoLeft);
                                                    if (currentAmmo != lastAmmo) {
                                                        if (currentAmmo < lastAmmo) {
                                                            int diff = lastAmmo - currentAmmo;
                                                            for (int i = 0; i < diff; ++i) { silentCounter++; if (silentCounter >= totalPattern) silentCounter = 0; }
                                                        }
                                                        lastAmmo = currentAmmo;
                                                    }
                                                }

                                                if (totalPattern > 0 && silentCounter < totalPattern && pPlayer.silen4a) aimHead = pattern[silentCounter];
                                                else aimHead = (headRate > 0);

                                                for (int x = 0; x < numValues; x++) {
                                                    uintptr_t entryAddr = valuesPtr + (x * 0x18);
                                                    uintptr_t enemyList = Read<uintptr_t>(entryAddr + 0x10);
                                                    if (enemyList != 0) {
                                                        if (enemyList == localPlayer) continue;
                                                        bool isClientBot = Read<bool>(enemyList + currentOffsets->isClientBot);
                                                        if (isClientBot == 1 || isClientBot == 0) {
                                                            uintptr_t AvatarManager = Read<uintptr_t>(enemyList + currentOffsets->avatarManager);
                                                            if (AvatarManager != 0) {
                                                                uintptr_t UmaAvatarSimple = Read<uintptr_t>(AvatarManager + currentOffsets->umaAvatarSimple);
                                                                if (UmaAvatarSimple != 0) {
                                                                    int specialStatus = Read<int>(enemyList + currentOffsets->TransformType);
                                                                    TransType currentAbility = static_cast<TransType>(specialStatus);
                                                                    bool isSpecial = (currentAbility == TransType::Wukong || currentAbility == TransType::Oreon || currentAbility == TransType::Ball || currentAbility == TransType::WukongSpecial);

                                                                    bool IsVisible = Read<bool>(UmaAvatarSimple + currentOffsets->isVisible);
                                                                    if (IsVisible == 1 || isSpecial) {
                                                                        uintptr_t UmaData = Read<uintptr_t>(UmaAvatarSimple + currentOffsets->umaData);
                                                                        if (UmaData != 0) {
                                                                            uintptr_t HHCBNAPCKHF = Read<uintptr_t>(enemyList + currentOffsets->ShadowState);
                                                                            if (HHCBNAPCKHF != 0) {
                                                                                bool isDieing = false; int State = Read<int>(HHCBNAPCKHF + currentOffsets->XPose); if (State == 8) isDieing = true;
                                                                                if (Read<bool>(UmaData + currentOffsets->isTeam) != 0) continue;
                                                                                if (Read<bool>(enemyList + currentOffsets->isDead) != 0) { g_isScaled.erase(enemyList); SetEnemyScale(enemyList,false); continue; }
                                                                                SetEnemyScale(enemyList, pPlayer.ScaleDown);

                                                                                int Health = 0; uintptr_t GetCurHP = Read<uintptr_t>(enemyList + currentOffsets->PlayerData);
                                                                                if (GetCurHP != 0) {
                                                                                    uintptr_t GetCur = Read<uintptr_t>(GetCurHP + 0x10);
                                                                                    if (GetCur != 0) {
                                                                                        uintptr_t HP = Read<uintptr_t>(GetCur + 0x20);
                                                                                        if (HP != 0) Health = (int)Read<short>(HP + currentOffsets->Health);
                                                                                    }
                                                                                }
                                                                                
                                                                                bool isSniperOrMarksman = false;
                                                                                switch ((short)localWeaponId) {
                                                                                    case 4: case 65: case 21: case 64: case 128: case 129: case 45: case 75: case 197: case 18: case 26: case 72: case 48: case 89: case 21006: case 21007: case 21008: case 21009: case 63: case 11: case 126: case 127 : case 78: case 805: case 806: case 807: case 7858: case 7824: case 7820: case 7823: case 7822: case 7817: case 7801: case 7818: case 7825:
                                                                                        isSniperOrMarksman = true; break;
                                                                                    default: break;
                                                                                }

                                                                                uintptr_t headTransform = GetPlayerHeadTF(enemyList);
                                                                                Vector3 EnemyPosition = GetNodePosition(headTransform);
                                                                                Vector3 EnemyPositionHip = GetNodePosition(GetPlayerHipTF(enemyList));
                                                                                Vector3 EnemyPositionPe = GetNodePosition(GetPlayerPeTF(enemyList));

                                                                                if (camPos == Vector3::Zero()) camPos = LocalPosition;
                                                                                float distanceToEnemy = Vector3::Distance(camPos, EnemyPosition);
                                                                                if (EnemyPosition == Vector3::Zero() || distanceToEnemy > 250.0f) continue;

                                                                                Vector3 viewForward = { matrix._13, matrix._23, matrix._33 }; Vector3 toEnemy = EnemyPosition - camPos;
                                                                                float dotProduct = toEnemy.X * viewForward.X + toEnemy.Y * viewForward.Y + toEnemy.Z * viewForward.Z;

                                                                                if (dotProduct >= 0.0f) SendResponse.EnemiesInFront++; else SendResponse.EnemiesBehind++;

                                                                                Vector3 screenPos = WorldToScreenPoint(matrix, EnemyPosition);
                                                                                float deltaX = screenPos.X - screenCenterX; float deltaY = screenPos.Y - screenCenterY;
                                                                                float distToCrosshair = sqrt(deltaX * deltaX + deltaY * deltaY);
                                                                                float fovRadius = (pPlayer.AimFov / 180.0f) * (g_screenWidth * 0.55f);

                                                                                bool shouldIgnoreForAim = (pPlayer.IgnoreKnock && isDieing);
                                                                                if (!shouldIgnoreForAim && !isLocalWeaponBlocked && distanceToEnemy <= 250.0f) {
                                                                                    if (distToCrosshair < bestAimLagDist) { bestAimLagDist = distToCrosshair; bestAimLagObj = enemyList; bestAimLagPos = EnemyPositionPe; }

                                                                                    if (screenPos.X >= 0 && screenPos.X <= g_screenWidth && screenPos.Y >= 0 && screenPos.Y <= g_screenHeight) {
                                                                                        bool fovPass = true;
                                                                                        if (pPlayer.silen4a || pPlayer.aimhard) fovPass = isWithinFOV(screenPos, fovRadius);

                                                                                        if (fovPass && distToCrosshair < bestDistance) {
                                                                                            bestDistance = distToCrosshair; bestTargetObj = enemyList; hasValidTarget = true;

                                                                                            Vector3 CalculatedChestPos;
                                                                                            CalculatedChestPos.X = EnemyPosition.X + (EnemyPositionHip.X - EnemyPosition.X) * 0.35f;
                                                                                            CalculatedChestPos.Y = EnemyPosition.Y + (EnemyPositionHip.Y - EnemyPosition.Y) * 0.35f;
                                                                                            CalculatedChestPos.Z = EnemyPosition.Z + (EnemyPositionHip.Z - EnemyPosition.Z) * 0.35f;

                                                                                            if (pPlayer.KillInChest && isSniperOrMarksman) { finalSilentPos = CalculatedChestPos; finalAimHardBaseHead = CalculatedChestPos; } 
                                                                                            else if (pPlayer.KillInChest && Health < 50 && Health > 0) { finalSilentPos = CalculatedChestPos; finalAimHardBaseHead = CalculatedChestPos; } 
                                                                                            else {
                                                                                                if (aimHead) { finalSilentPos = EnemyPosition; finalAimHardBaseHead = EnemyPosition; } 
                                                                                                else { finalSilentPos = CalculatedChestPos; finalAimHardBaseHead = CalculatedChestPos; }
                                                                                            }
                                                                                            finalAimHardBaseHip = EnemyPositionHip;
                                                                                        }
                                                                                    }

                                                                                    float pullDeltaX = screenPos.X - screenCenterX; float pullDeltaY = screenPos.Y - screenCenterY;
                                                                                    float distToPullCrosshair = sqrt(pullDeltaX * pullDeltaX + pullDeltaY * pullDeltaY);
                                                                                    float pullFovRadius = (360.0f / 180.0f) * (g_screenHeight / 2.0f);

                                                                                    if (pPlayer.pullenemy && isWithinPullFOV(screenPos, pullFovRadius) && distanceToEnemy <= 250.0f) {
                                                                                        if (pPlayer.IgnoreKnock && isDieing) continue;
                                                                                        if (distToPullCrosshair < bestPullCrosshairDist) { bestPullCrosshairDist = distToPullCrosshair; bestPullTargetObj = enemyList; hasPullTarget = true; }
                                                                                    }
                                                                                }

                                                                                Vector3 LocationHeadBox = WorldToScreenPoint(matrix, EnemyPosition); Vector3 LocationToeBox = WorldToScreenPoint(matrix, EnemyPositionPe);
                                                                                if (LocationHeadBox.X < -0 || LocationToeBox.X < -0) continue;

                                                                                PlayerData* data = &SendResponse.Players[SendResponse.PlayerCount];
                                                                                memset(data->PlayerName, 0, 64);
                                                                                if (isClientBot) strcpy(data->PlayerName, "Bot");
                                                                                else {
                                                                                    uintptr_t NamePtr = Read<uintptr_t>(enemyList + currentOffsets->PlayerName);
                                                                                    if (NamePtr > 0x10000000) getUTF8(data->PlayerName, NamePtr);
                                                                                    if (data->PlayerName[0] == '\0') strcpy(data->PlayerName, "Traning Bot");
                                                                                }

                                                                                data->SpecialAbilities = currentAbility; data->Head = LocationHeadBox; data->Toe = LocationToeBox;
                                                                                data->IsCaido = isDieing; data->Distancia = distanceToEnemy;
                                                                                data->Name = isClientBot; data->Health = Health; data->WeaponId = (short)ReadCurrentWeapon(enemyList);

                                                                                if (isSpecial && (data->Head.X == 0 || data->Head.Y == 0)) {
                                                                                    uintptr_t hipTF = GetPlayerHipTF(enemyList); data->Head = GetNodePosition(hipTF); data->Head.Y -= 0.5f;
                                                                                }

                                                                                ++SendResponse.PlayerCount;
                                                                                if (SendResponse.PlayerCount == maxplayerCount) break;
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }

                                                uintptr_t Atri = Read<uintptr_t>(livePlayer + currentOffsets->Attributes);

                                                if (pPlayer.pullenemy) {
                                                    if (hasPullTarget) { g_pullEnemyTargetAddr = bestPullTargetObj; g_hasPullTargetV2 = true; } 
                                                    else { g_pullEnemyTargetAddr = 0; g_hasPullTargetV2 = false; }
                                                }

                                                if (pPlayer.PCMovement && Actived.activar) {
                                                    uintptr_t UserControl = Read<uintptr_t>(livePlayer + currentOffsets->UserControl);
                                                    if (UserControl) { Write<int>(UserControl + 0x90, 1); Write<bool>(UserControl + 0x9C, true); oriPc = true; }
                                                } else if (oriPc) {
                                                    uintptr_t UserControl = Read<uintptr_t>(livePlayer + currentOffsets->UserControl);
                                                    if (UserControl) { Write<int>(UserControl + 0x90, 0); Write<bool>(UserControl + 0x9C, false); }
                                                    oriPc = false;
                                                }

                                                if (pPlayer.SpeedTimer && Actived.activar) {
                                                    if (!originalSpeedRead) { OriSpeed = Read<float>(Atri + currentOffsets->ForceSpeed); originalSpeedRead = true; }
                                                    Write<float>(Atri + currentOffsets->ForceSpeed, 6.8f); SpeedAct = true;
                                                } else if (SpeedAct) { Write<float>(Atri + currentOffsets->ForceSpeed, OriSpeed); OriSpeed = 0.0f; SpeedAct = false; originalSpeedRead = false; }

                                                if (pPlayer.GhostHack && Actived.activar) { Write(livePlayer + currentOffsets->GhostHack,true); GhostAct = true; } 
                                                else if(GhostAct){ Write(livePlayer + currentOffsets->GhostHack,false); GhostAct = false; }

                                                if (pPlayer.aimhard && hasValidTarget && livePlayer != 0 && !isLocalWeaponBlocked) {
                                                    bool isFiring = Read<bool>(livePlayer + currentOffsets->isFiring);
                                                    auto Weapon = Read<uintptr_t>(livePlayer + currentOffsets->Weapon);
                                                    bool isSighted = false;
                                                    if (Weapon) { uintptr_t sightPtr = Read<uintptr_t>(Weapon + currentOffsets->isSight); isSighted = (sightPtr != 0); }

                                                    if (isFiring || isSighted) {
                                                        Vector3 targetToLock;
                                                        if (pPlayer.KillInChest && (finalAimHardBaseHead == finalSilentPos)) targetToLock = finalAimHardBaseHead;
                                                        else if (pPlayer.ChestRate <= 0.0f) targetToLock = finalAimHardBaseHead;
                                                        else {
                                                            static int aimTickCounter = 0; aimTickCounter++; if (aimTickCounter > 100) aimTickCounter = 0;
                                                            float threshold = pPlayer.ChestRate * 10.0f;
                                                            if ((float)aimTickCounter < threshold) {
                                                                targetToLock.X = finalAimHardBaseHead.X + (finalAimHardBaseHip.X - finalAimHardBaseHead.X) * 0.35f;
                                                                targetToLock.Y = finalAimHardBaseHead.Y + (finalAimHardBaseHip.Y - finalAimHardBaseHead.Y) * 0.35f;
                                                                targetToLock.Z = finalAimHardBaseHead.Z + (finalAimHardBaseHip.Z - finalAimHardBaseHead.Z) * 0.35f;
                                                            } else targetToLock = finalAimHardBaseHead;
                                                        }
                                                        std::lock_guard<std::mutex> lock(g_aimHardMutex);
                                                        g_aimHardTargetObj = bestTargetObj; g_aimHardTargetPos = targetToLock; g_hasAimHardTarget = true;
                                                    } else { std::lock_guard<std::mutex> lock(g_aimHardMutex); g_hasAimHardTarget = false; }
                                                } else { std::lock_guard<std::mutex> lock(g_aimHardMutex); g_hasAimHardTarget = false; }

                                                if (pPlayer.AimLag) {
                                                    g_aimLagTargetObj = bestAimLagObj; g_aimLagTargetPos = bestAimLagPos; g_hasAimLagTarget = (bestAimLagObj != 0);
                                                } else { g_aimLagTargetObj = 0; g_aimLagTargetPos = Vector3::Zero(); g_hasAimLagTarget = false; }

                                                if (pPlayer.aimbotlock && hasValidTarget && livePlayer != 0 && !isLocalWeaponBlocked) {
                                                    g_aimbotTargetObj = bestTargetObj; g_aimbotTargetPos = finalSilentPos;
                                                } else { g_aimbotTargetObj = 0; g_aimbotTargetPos = Vector3::Zero(); }

                                                if (pPlayer.silen4a && hasValidTarget && livePlayer != 0 && !isLocalWeaponBlocked) {
                                                    currentBestTargetPos = finalSilentPos; currentBestTargetObj = bestTargetObj; keepAiming = true;
                                                } else { keepAiming = false; currentBestTargetPos = Vector3::Zero(); }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (SendResponse.PlayerCount > 0) {
        lastValidResponse = SendResponse; framesWithoutPlayers = 0;
    } else {
        framesWithoutPlayers++;
        if (framesWithoutPlayers <= MAX_FRAMES_WITHOUT_PLAYERS && lastValidResponse.PlayerCount > 0) SendResponse = lastValidResponse;
        else { lastValidResponse = {}; framesWithoutPlayers = 0; }
    }
}

__attribute__ ((visibility ("hidden")))
int main(int argc, char*argv[]){
    if (InitServer() == (0)) {
        if (server.Accept()) {
            target_pid = FindPid("com.dts.freefiremax");
            if (target_pid > 0) {
                // Rely on HMKPM
            } else { exit(1); }

            libAddress = FindCorrectLibBase();
            std::thread(SilentAimTh).detach();
            std::thread(FlyHackTh).detach();
            std::thread(AimColliderTh).detach();
            std::thread(AimHardTh).detach();
            std::thread(PullEnemyTh).detach();
            std::thread(DiveKillThread).detach();
            std::thread(RotateTh).detach();
            std::thread(TeleportToMarkThread).detach();

            Request request{};
            while (server.receive((void*)&request) > (0)) {
                Response SendResponse{};
                if (request.Mode == Mode::InitMode) SendResponse.Success = true;
                else if (request.Mode == Mode::HackMode) SendResponse.Success = true;
                else if (request.Mode == Mode::EspMode) {
                    g_screenWidth = request.ScreenWidth; g_screenHeight = request.ScreenHeight; CreateDataList(SendResponse); SendResponse.Success = true;
                } else if (request.Mode == 3) { Actived.activar = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 41){ pPlayer.FlyHack = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 42){ pPlayer.PCMovement = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 43){ pPlayer.TPMark = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 44){ pPlayer.pullenemy = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 45){ pPlayer.KillInChest = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 48){ pPlayer.GhostHack = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 49){ pPlayer.WallBug = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 50){ pPlayer.SpinBot = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 51){ pPlayer.aimhard = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 52){ pPlayer.SpeedTimer = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 53){ pPlayer.ScaleDown = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 54){ pPlayer.aimbotlock = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 55){ pPlayer.IgnoreKnock = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 57){ pPlayer.silen4a = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 58){ pPlayer.AimFov = (float)request.value; SendResponse.Success = true; }
                else if(request.Mode == 59){ pPlayer.DiveKill = request.m_IsOn; SendResponse.Success = true; }
                else if(request.Mode == 62){ pPlayer.ChestRate = (float)request.value; SendResponse.Success = true; }
                else if(request.Mode == 70){ pPlayer.HeadRate = (float)request.value; SendResponse.Success = true; }
                else if (request.Mode == 100) {
                    if (request.value == 1) currentOffsets = &FFMaxInd; else currentOffsets = &FFMax;
                    SendResponse.Success = true;
                }
                server.send((void*)& SendResponse, sizeof(SendResponse));
            }
        }
    }
    return 0;
}
