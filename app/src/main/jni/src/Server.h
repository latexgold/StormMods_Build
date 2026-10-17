#include <jni.h>
#include <stdio.h>
#include <wchar.h>
#include "Includes/Logger.h"
#include "src/Unity/Quaternion.hpp"
#include "src/Unity/Vector3.hpp"

#include <src/Socket/server.h>
#include "src/Unity/Unity.h"
#include "src/Unity/Color.hpp"
#include "src/Unity/Rect.hpp"
#include "src/Unity/ESP.h"

#include <stdint.h>
#include "Includes/Utils.h"
#include "BooleanServer.h"
#include <thread>
#include <chrono>
#include "Offsets.h"
SocketServer server;

int g_screenWidth, g_screenHeight;

#define maxplayerCount 100

enum Mode {
    InitMode = 1,
    HackMode = 2,
    StopMode = 98,
    EspMode = 99
};

struct Request {
    int Mode;
    bool m_IsOn;
    int value;
    int ScreenWidth;
    int ScreenHeight;
};

enum class TransType : int {
    None = 0,
    Wukong = 1,
    Oreon = 2,
    Ball = 3,
    WukongSpecial = 4
};

struct PlayerData {
    Vector3 Head;
    Vector3 Toe;
    bool IsCaido;
    float Distancia;
    int Health;
    bool Name;
    char PlayerName[64];
    TransType SpecialAbilities;
    short WeaponId;
};

struct Response {
    bool Success;
    int PlayerCount;
    int EnemiesInFront;
    int EnemiesBehind;
    PlayerData Players[maxplayerCount];
};

int InitServer() {
    if (!server.Create()) {
        LOGE("[Server] Socket can't connect");
        return -1;
    }
    if (!server.Bind()) {
        LOGE("[Server] Socket can't connect");
        return -1;
    }
    if (!server.Listen()) {
        LOGE("[Server] Socket can't connect");
        return -1;
    }
    LOGI("[Server] Socket connected");
    return 0;
}

uintptr_t GetPlayerHeadTF(uintptr_t player) {
    auto ptr = Read<uintptr_t>(player + currentOffsets->HeadBone);
    return ptr;
}

uintptr_t GetPlayerHipTF(uintptr_t player) {
    auto ptr = Read<uintptr_t>(player + currentOffsets->HipBone);
    return ptr;
}


uintptr_t GetPlayerPeTF(uintptr_t player) {
    auto ptr = Read<uintptr_t>(player + currentOffsets->RootBone);
    return ptr;
}

struct Matrix {
    Vector4 Position;
    Quaternion Rotation;
    Vector4 Scale;
};

static auto GetPosition(uintptr_t Transform) {
    auto pos = Vector3::Zero();

    auto transformObjValue = Read<uintptr_t>(Transform + 0x10);
    if (transformObjValue == 0) {
        return pos;
    }

    auto indexValue = Read<uintptr_t>(transformObjValue + 0x40);
    auto matrixValue = Read<uintptr_t>(transformObjValue + 0x38);

    auto matrixListValue = Read<uintptr_t>(matrixValue + 0x18);
    auto matrixIndicesValue = Read<uintptr_t>(matrixValue + 0x20);

    auto resultValue = Read<Vector3>(matrixListValue + (indexValue * 0x30)); // FIXED: 64-bit pointer arithmetic

    auto maxTries = 50;
    auto tries = 0;

    // FIXED: Proper 64-bit pointer arithmetic
    auto transformIndexValue = Read<int>(matrixIndicesValue + (indexValue * 4));

    while (transformIndexValue >= 0 && tries < maxTries) {
        tries++;

        // FIXED: Proper 64-bit pointer arithmetic
        auto tMatrixValue = Read<Matrix>(matrixListValue + (transformIndexValue * 0x30));

        auto rotX = tMatrixValue.Rotation.X;
        auto rotY = tMatrixValue.Rotation.Y;
        auto rotZ = tMatrixValue.Rotation.Z;
        auto rotW = tMatrixValue.Rotation.W;

        auto scaleX = resultValue.X * tMatrixValue.Scale.X;
        auto scaleY = resultValue.Y * tMatrixValue.Scale.Y;
        auto scaleZ = resultValue.Z * tMatrixValue.Scale.Z;

        resultValue.X = tMatrixValue.Position.X + scaleX +
                        (scaleX * ((rotY * rotY * -2.0) - (rotZ * rotZ * 2.0))) +
                        (scaleY * ((rotW * rotZ * -2.0) - (rotY * rotX * -2.0))) +
                        (scaleZ * ((rotZ * rotX * 2.0) - (rotW * rotY * -2.0)));
        resultValue.Y = tMatrixValue.Position.Y + scaleY +
                        (scaleX * ((rotX * rotY * 2.0) - (rotW * rotZ * -2.0))) +
                        (scaleY * ((rotZ * rotZ * -2.0) - (rotX * rotX * 2.0))) +
                        (scaleZ * ((rotW * rotX * -2.0) - (rotZ * rotY * -2.0)));
        resultValue.Z = tMatrixValue.Position.Z + scaleZ +
                        (scaleX * ((rotW * rotY * -2.0) - (rotX * rotZ * -2.0))) +
                        (scaleY * ((rotY * rotZ * 2.0) - (rotW * rotX * -2.0))) +
                        (scaleZ * ((rotX * rotX * -2.0) - (rotY * rotY * 2.0)));

        // FIXED: Proper 64-bit pointer arithmetic
        transformIndexValue = Read<int>(matrixIndicesValue + (transformIndexValue * 4));
    }

    if (tries < maxTries) {
        pos = resultValue;
    }

    return pos;
}

static auto GetNodePosition(uintptr_t nodeTransform) {
    auto transformValue = Read<uintptr_t>(nodeTransform + 0x10);
    if (transformValue == 0) {
        return Vector3::Zero();
    }
    return GetPosition(transformValue);
}

static auto WorldToScreenPoint(D3DMatrix viewMatrix, Vector3 ScreenPos) {
    auto result = Vector3(-1, -1, -1);

    auto v9 = (ScreenPos.X * viewMatrix._11) + (ScreenPos.Y * viewMatrix._21) + (ScreenPos.Z * viewMatrix._31) + viewMatrix._41;
    auto v10 = (ScreenPos.X * viewMatrix._12) + (ScreenPos.Y * viewMatrix._22) + (ScreenPos.Z * viewMatrix._32) + viewMatrix._42;
    auto v12 = (ScreenPos.X * viewMatrix._14) + (ScreenPos.Y * viewMatrix._24) + (ScreenPos.Z * viewMatrix._34) + viewMatrix._44;

    if (v12 >= 0.001f) {
        auto v13 = (float)g_screenWidth / 2.0f;
        auto v14 = (float)g_screenHeight / 2.0f;

        result.X = v13 + (v13 * v9) / v12;
        result.Y = v14 - (v14 * v10) / v12;
    }
    return result;
}