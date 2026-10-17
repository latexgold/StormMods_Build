#pragma once
#include <android/log.h>
#include <math.h>
#include <stdint.h>
#include "Vector3.hpp"
#include "Vector2.hpp"
#include "Quaternion.hpp"

// Prevent redefinition warnings
#ifndef LOG_TAG
#define LOG_TAG  "DAEMON"
#endif

#ifndef LOGE
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#endif

// --- Math Helpers ---

float NormalizeAngle (float angle){
    while (angle > 360) angle -= 360;
    while (angle < 0) angle += 360;
    return angle;
}

Vector3 NormalizeAngles (Vector3 angles){
    angles.X = NormalizeAngle (angles.X);
    angles.Y = NormalizeAngle (angles.Y);
    angles.Z = NormalizeAngle (angles.Z);
    return angles;
}

Vector3 ToEulerRad(Quaternion q1){
    float Rad2Deg = 360.0 / (M_PI * 2.0);
    float sqw = q1.W * q1.W;
    float sqx = q1.X * q1.X;
    float sqy = q1.Y * q1.Y;
    float sqz = q1.Z * q1.Z;
    float unit = sqx + sqy + sqz + sqw;
    float test = q1.X * q1.W - q1.Y * q1.Z;
    Vector3 v;

    if (test > 0.4995 * unit) {
        v.Y = 2.0 * atan2f (q1.Y, q1.X);
        v.X = M_PI / 2.0;
        v.Z = 0;
        return NormalizeAngles(v * Rad2Deg);
    }
    if (test < -0.4995 * unit) {
        v.Y = -2.0 * atan2f (q1.Y, q1.X);
        v.X = -M_PI / 2.0;
        v.Z = 0;
        return NormalizeAngles (v * Rad2Deg);
    }
    Quaternion q(q1.W, q1.Z, q1.X, q1.Y);
    v.Y = atan2f (2.0 * q.X * q.W + 2.0 * q.Y * q.Z, 1 - 2.0 * (q.Z * q.Z + q.W * q.W));
    v.X = asinf (2.0 * (q.X * q.Z - q.W * q.Y));
    v.Z = atan2f (2.0 * q.X * q.Y + 2.0 * q.Z * q.W, 1 - 2.0 * (q.Y * q.Y + q.Z * q.Z));
    return NormalizeAngles (v * Rad2Deg);
}

Quaternion GetRotationToLocation(Vector3 targetLocation, float y_bias, Vector3 myLoc){
    return Quaternion::LookRotation((targetLocation + Vector3(0, y_bias, 0)) - myLoc, Vector3(0, 1, 0));
}

// --- IL2CPP Structures ---

template <typename T>
struct monoArray
{
    void* klass;
    void* monitor;
    void* bounds;
    int   max_length;
    T vector [1]; // Corrected: Should be type T, not void*

    int getLength() {
        return (this != nullptr) ? max_length : 0;
    }
    T* getPointer() {
        return vector;
    }
};

template <typename T>
struct monoList {
    void *unk0;
    void *unk1;
    monoArray<T> *items;
    int size;
    int version;

    T* getItems(){
        return (items != nullptr) ? items->getPointer() : nullptr;
    }
    int getSize(){
        return size;
    }
};

template <typename K, typename V>
struct monoDictionary {
    void *unk0;
    void *unk1;
    void *table;
    void *linkSlots;
    monoArray<K> *keys;
    monoArray<V> *values;
    int touchedSlots;
    int emptySlot;
    int size;

    K* getKeys(){
        return (keys != nullptr) ? keys->getPointer() : nullptr;
    }
    V* getValues(){
        return (values != nullptr) ? values->getPointer() : nullptr;
    }
    int getNumKeys(){
        return (keys != nullptr) ? keys->getLength() : 0;
    }
    int getNumValues(){
        return (values != nullptr) ? values->getLength() : 0;
    }
};

union intfloat {
	int i;
	float f;
};

typedef struct _monoString
{
    void* klass;
    void* monitor;
    int length;    
    char16_t chars[1]; // Unity strings are UTF-16
    
    int getLength() { return length; }
    char16_t* getChars() { return chars; }
} monoString;

// --- Obscured Types ---

int GetObscuredIntValue(uintptr_t location){
    if (!location) return 0;
	int cryptoKey = *(int *)location;
	int obfuscatedValue = *(int *)(location + 0x4);
	return obfuscatedValue ^ cryptoKey;
}

void SetObscuredIntValue(uintptr_t location, int value){
    if (!location) return;
	int cryptoKey = *(int *)location;
	*(int *)(location + 0x4) = value ^ cryptoKey;
}

float GetObscuredFloatValue(uintptr_t location){
    if (!location) return 0;
	int cryptoKey = *(int *)location;
	int obfuscatedValue = *(int *)(location + 0x4);
	intfloat IF;
	IF.i = obfuscatedValue ^ cryptoKey;
	return IF.f;
}

void SetObscuredFloatValue(uintptr_t location, float value){
    if (!location) return;
	int cryptoKey = *(int *)location;
	intfloat IF;
	IF.f = value;
	intfloat IF2;
	IF2.i = IF.i ^ cryptoKey;
	*(float *)(location + 0x4) = IF2.f;
}
