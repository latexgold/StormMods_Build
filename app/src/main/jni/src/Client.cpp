#include <pthread.h>
#include <jni.h>
#include <src/Socket/client.h>
#include "src/Includes/obfuscate.h"
#include <sys/stat.h>
#include <src/Widgets/ImportWidgets.h>
#include <src/Includes/http.h>
#include "src/Unity/Vector3.hpp"
#include "src/Unity/Vector2.hpp"
#include "src/Unity/Unity.h"
#include "src/Unity/ESP.h"
#include "Includes/Logger.h"
#include "Includes/Utils.h"
#include "src/Widgets/Methods.h"
#include <cmath>
#include "Client.h"
#include "BooleanClient.h"

#include <curl/curl.h>
#include "src/Includes/json.h"
#include <jni.h>
#include <string>
#include <android/log.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <errno.h>
#include "Namegun.h"

extern bool g_useKernelDriver;

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_LoginActivity_setKernelDriverMode(JNIEnv *env, jclass clazz, jboolean enable) {
    g_useKernelDriver = enable;
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_LoginActivity_setGameVersionNative(JNIEnv *env, jclass clazz, jstring version) {
    const char *versionStr = env->GetStringUTFChars(version, nullptr);
    g_selectedVersion = std::string(versionStr);
    env->ReleaseStringUTFChars(version, versionStr);
    if (isConnected()) {
        int index = (g_selectedVersion == "v2.133.x") ? 1 : 0;
        SendVersionToServer(index);
    }
}

using json = nlohmann::json;
bool showSpeed = false;
bool showDive = false;
bool showScale = false;
bool showMagnet = false;
bool showGhost = false;
bool showWall = false;
bool showMark = false;
bool showFly = false;

struct MemoryChunk {
    char *memory;
    size_t size;
};

int g_screenWidth, g_screenHeight;

struct {
    bool connectClient = false;
    std::string server_msg = "Waiting...";
} Connection;

ESP espOverlay;

std::string espname12 = "";
std::atomic<int> uisid2iuwaw(8261);
int kbjisui8821 = 0;

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_ChangesString(JNIEnv *env, jclass clazz, jint id, jstring value) {
    const char *nativeString = env->GetStringUTFChars(value, 0);
    if (id == 999) {
        espname12 = nativeString;
    }
    env->ReleaseStringUTFChars(value, nativeString);
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryChunk *mem = (struct MemoryChunk *)userp;
    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

uint64_t computeFileFingerprint(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return 0;

    uint64_t fingerprint = 14695981039346656037ULL;
    char byte;
    while (file.get(byte)) {
        fingerprint ^= (uint8_t)byte;
        fingerprint *= 1099511628211ULL;
    }
    file.close();
    return fingerprint;
}

std::string getApkPath(JNIEnv* env, jobject context) {
    jclass contextClass = env->GetObjectClass(context);
    jmethodID getPackageCodePath = env->GetMethodID(contextClass, "getPackageCodePath", "()Ljava/lang/String;");
    jstring pathJstr = (jstring)env->CallObjectMethod(context, getPackageCodePath);
    const char* pathCstr = env->GetStringUTFChars(pathJstr, nullptr);
    std::string apkPath(pathCstr);
    env->ReleaseStringUTFChars(pathJstr, pathCstr);
    env->DeleteLocalRef(pathJstr);
    env->DeleteLocalRef(contextClass);
    return apkPath;
}

std::string getPrivateCacheDir(JNIEnv* env, jobject context) {
    jclass contextClass = env->GetObjectClass(context);
    jmethodID getCacheDir = env->GetMethodID(contextClass, "getCacheDir", "()Ljava/io/File;");
    jobject cacheFile = env->CallObjectMethod(context, getCacheDir);
    jclass fileClass = env->GetObjectClass(cacheFile);
    jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring pathJstr = (jstring)env->CallObjectMethod(cacheFile, getAbsolutePath);
    const char* pathCstr = env->GetStringUTFChars(pathJstr, nullptr);
    std::string cachePath(pathCstr);
    env->ReleaseStringUTFChars(pathJstr, pathCstr);
    env->DeleteLocalRef(pathJstr);
    env->DeleteLocalRef(cacheFile);
    env->DeleteLocalRef(fileClass);
    env->DeleteLocalRef(contextClass);
    return cachePath + "/.hashdata/";
}

bool retrieveProtectedData(const std::string& url, const std::string& outputPath) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    FILE* fp = fopen(outputPath.c_str(), "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code != 200) {
        return false;
    }
    return true;
}

std::string decryptXor(const std::string& data, const std::string& key) {
    std::string result = data;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }
    return result;
}

bool ShowEspwea(JNIEnv* env, jobject context) {
    std::string apkPath = getApkPath(env, context);
    if (apkPath.empty()) {
        return false;
    }
    uint64_t actualHash = computeFileFingerprint(apkPath);
    if (actualHash == 0) {
        return false;
    }
    std::string hashDir = getPrivateCacheDir(env, context);
    if (mkdir(hashDir.c_str(), 0700) == -1 && errno != EEXIST) {
        return false;
    }
    std::string url = "Your_Url";
    std::string localFile = hashDir + "Name.bin";

    if (!retrieveProtectedData(url, localFile)) {
        return false;
    }
    std::ifstream file(localFile, std::ios::binary);
    if (!file) {
        return false;
    }
    std::string encryptedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    std::string decrypted = decryptXor(encryptedData, "Your_secret_key");
    if (decrypted.empty()) {
        return false;
    }
    uint64_t expectedHash = std::stoull(decrypted);
    if (actualHash == expectedHash) {
        return true;
    } else {
        return false;
    }
}

bool ShowApk(JNIEnv* env, jobject context) {
    std::string apkPath = getApkPath(env, context);
    if (apkPath.empty()) return false;

    jclass zipFileClass = env->FindClass("java/util/zip/ZipFile");
    if (!zipFileClass) return false;

    jmethodID zipCtor = env->GetMethodID(zipFileClass, "<init>", "(Ljava/lang/String;)V");
    jstring jPath = env->NewStringUTF(apkPath.c_str());
    jobject zipFile = env->NewObject(zipFileClass, zipCtor, jPath);
    env->DeleteLocalRef(jPath);

    if (env->ExceptionCheck() || !zipFile) {
        env->ExceptionClear();
        env->DeleteLocalRef(zipFileClass);
        return false;
    }

    jmethodID entriesMethod = env->GetMethodID(zipFileClass, "entries", "()Ljava/util/Enumeration;");
    jobject entries = env->CallObjectMethod(zipFile, entriesMethod);
    if (env->ExceptionCheck() || !entries) {
        env->ExceptionClear();
        env->DeleteLocalRef(zipFile);
        env->DeleteLocalRef(zipFileClass);
        return false;
    }

    jclass enumClass = env->FindClass("java/util/Enumeration");
    jmethodID hasMore = env->GetMethodID(enumClass, "hasMoreElements", "()Z");
    jmethodID nextElem = env->GetMethodID(enumClass, "nextElement", "()Ljava/lang/Object;");
    jclass zipEntryClass = env->FindClass("java/util/zip/ZipEntry");
    jmethodID getName = env->GetMethodID(zipEntryClass, "getName", "()Ljava/lang/String;");

    bool foundBKLSEC = false;
    bool hasOtherLibrary = false;
    int dexCount = 0;

    while (env->CallBooleanMethod(entries, hasMore)) {
        if (env->ExceptionCheck()) { env->ExceptionClear(); break; }
        jobject entryObj = env->CallObjectMethod(entries, nextElem);
        if (env->ExceptionCheck() || !entryObj) { env->ExceptionClear(); break; }

        jstring entryNameJstr = (jstring)env->CallObjectMethod(entryObj, getName);
        if (env->ExceptionCheck() || !entryNameJstr) { env->ExceptionClear(); env->DeleteLocalRef(entryObj); continue; }

        const char* entryName = env->GetStringUTFChars(entryNameJstr, nullptr);
        std::string name(entryName);
        env->ReleaseStringUTFChars(entryNameJstr, entryName);
        env->DeleteLocalRef(entryNameJstr);
        env->DeleteLocalRef(entryObj);

        if (name.find("lib/") == 0 && name.find(".so") != std::string::npos) {
            size_t lastSlash = name.find_last_of('/');
            std::string baseName = (lastSlash != std::string::npos) ? name.substr(lastSlash + 1) : name;

            if (baseName == "libBKLSEC.so") {
                foundBKLSEC = true;
            } else {
                hasOtherLibrary = true;
            }
        }
        else if (name.length() >= 4 && name.substr(name.length() - 4) == ".dex") {
            dexCount++;
        }
    }

    env->DeleteLocalRef(entries);
    env->DeleteLocalRef(zipFile);
    env->DeleteLocalRef(zipFileClass);
    env->DeleteLocalRef(enumClass);
    env->DeleteLocalRef(zipEntryClass);

    if (!foundBKLSEC) return false;
    if (hasOtherLibrary) return false;
    if (dexCount > 3) return false;

    return true;
}

bool RunCheck(JNIEnv* env, jobject context) {
    if (!ShowEspwea(env, context)) {
        return false;
    }
    if (!ShowApk(env, context)) {
        return false;
    }
    return true;
}

void DrawESP(ESP esp) {
    auto screenHeight = esp.getHeight();
    auto screenWidth = esp.getWidth();
    if (!isConnected()) return;

    if (pEspPlayer.Info) {
        float bottomY = screenHeight - 20.0f;
        float textSize = 15.0f;
        /*const char* watermark = "BKL SENSI v2.1.2 Debug";
        if (g_selectedVersion == "v2.131.x"){
            watermark = "BKL SENSI v2.1.2 Debug India";
        } else {
            watermark = "BKL SENSI v2.1.2 Debug Global";
        }*/
        std::string baseWatermark = (g_selectedVersion == "v2.133.x")
                                    ? "STORM MODS / India"
                                    : "STORM MODS / Global";
        std::string methodText = g_useKernelDriver ? "Kernel" : "Root";
        std::string fullWatermark = baseWatermark + " / " + methodText;
        const char* wm = fullWatermark.c_str();
        esp.DrawText(Color(0,0,0,255), wm, Vector2(screenWidth/2 - 1, bottomY - 1), textSize);
        esp.DrawText(Color(0,0,0,255), wm, Vector2(screenWidth/2 + 1, bottomY + 1), textSize);
        esp.DrawText(Color(0,0,0,255), wm, Vector2(screenWidth/2 - 1, bottomY + 1), textSize);
        esp.DrawText(Color(0,0,0,255), wm, Vector2(screenWidth/2 + 1, bottomY - 1), textSize);
        esp.DrawText(Color(255,255,255,255), wm, Vector2(screenWidth/2, bottomY), textSize);
    }
    if (kbjisui8821 != 9251) return;

    if (Actived.activar && pEspPlayer.drawFov && pPlayer.AimFov > 0) {
        float centerX = screenWidth / 2.0f;
        float centerY = screenHeight / 2.0f;
        float fovRadius = (pPlayer.AimFov / 180.0f) * (screenWidth * 0.55f);

        Color fovColor = pEspPlayer.espColor;
        esp.DrawCircle(fovColor, 1.5f, Vector2(centerX, centerY), fovRadius);
    }

    auto response = getData(screenWidth, screenHeight);
    if (!response.Success && response.EnemiesInFront == 0 && response.EnemiesBehind == 0) {
        return;
    }

    float globalThickness = 1.5f;
    Color whiteColor = Color(255, 255, 255, 255);
    Color strokeColor = Color(0, 0, 0, 255);
    float fSize = pEspPlayer.textSize;

    if (Actived.activar && pEspPlayer.espCount) {
        float textX = screenWidth / 2.0f;
        float currentY = screenHeight * 0.15f;
        float countTextSize = fSize * 1.8f;

        if (response.EnemiesInFront > 0) {
            char frontText[32];
            sprintf(frontText, "Enemy: %d", response.EnemiesInFront);
            esp.DrawText(strokeColor, frontText, Vector2(textX - 1, currentY - 1), countTextSize);
            esp.DrawText(strokeColor, frontText, Vector2(textX + 1, currentY + 1), countTextSize);
            esp.DrawText(strokeColor, frontText, Vector2(textX - 1, currentY + 1), countTextSize);
            esp.DrawText(strokeColor, frontText, Vector2(textX + 1, currentY - 1), countTextSize);
            esp.DrawText(whiteColor, frontText, Vector2(textX, currentY), countTextSize);
            currentY += countTextSize + 10.0f;
        }

        if (response.EnemiesBehind > 0) {
            char backText[32];
            sprintf(backText, "Behind: %d", response.EnemiesBehind);
            esp.DrawText(strokeColor, backText, Vector2(textX - 1, currentY - 1), countTextSize);
            esp.DrawText(strokeColor, backText, Vector2(textX + 1, currentY + 1), countTextSize);
            esp.DrawText(strokeColor, backText, Vector2(textX - 1, currentY + 1), countTextSize);
            esp.DrawText(strokeColor, backText, Vector2(textX + 1, currentY - 1), countTextSize);
            esp.DrawText(whiteColor, backText, Vector2(textX, currentY), countTextSize);
        }
    }

    if (response.PlayerCount <= 0) return;

    for (int i = 0; i < response.PlayerCount; i++) {
        auto player = response.Players[i];
        bool isSpecial = (player.SpecialAbilities == TransType::Wukong || player.SpecialAbilities == TransType::Oreon || player.SpecialAbilities == TransType::Ball || player.SpecialAbilities == TransType::WukongSpecial);
        if (!Actived.activar) continue;

        float distance = player.Distancia;
        if (distance < 0.1f || distance > 250.0f) continue;

        if (isSpecial && player.Head.Y > player.Toe.Y - 10.0f) {
            player.Head.Y = player.Toe.Y - 60.0f;
        }
        if (player.Head == Vector3::Zero() || player.Toe == Vector3::Zero()) continue;

        //Positioning
        float headToFeetHeight = std::abs(player.Toe.Y - player.Head.Y);
        float boxHeight = headToFeetHeight * 1.15f;
        float width = boxHeight * 0.48f;
        float boxY = player.Head.Y - (boxHeight * 0.12f);
        float boxX = player.Head.X - (width / 2.0f);

        Color colorLine = pEspPlayer.espColor;
        Color colorBox = pEspPlayer.espColor;

        if (player.IsCaido) {
            colorLine = Color(255, 0, 0, 255);
            colorBox = Color(255, 0, 0, 255);
        } else {
            if (player.SpecialAbilities == TransType::Wukong || player.SpecialAbilities == TransType::WukongSpecial) {
                colorLine = Color(0, 255, 0, 255);
            } else if (player.SpecialAbilities == TransType::Oreon) {
                colorLine = Color(130, 0, 0, 255);
            }
            else if (player.SpecialAbilities == TransType::Ball) {
                colorLine = Color(200, 200, 200, 255);
            }
        }

        //Draw Line
        if (pEspPlayer.espLinha) {
            Vector3 lineStart, lineEnd;
            if (pEspPlayer.lineType == LINE_TOP) lineStart = Vector3(screenWidth / 2, 0, 0);
            else if (pEspPlayer.lineType == LINE_CENTER) lineStart = Vector3(screenWidth / 2, screenHeight / 2, 0);
            else lineStart = Vector3(screenWidth / 2, screenHeight, 0);

            lineEnd = (pEspPlayer.lineType == LINE_BOTTOM) ? Vector3(player.Head.X, boxY + boxHeight, 0) : Vector3(player.Head.X, boxY, 0);
            esp.DrawLine(colorLine, globalThickness, lineStart, lineEnd);
        }

        //Draw Box
        if (pEspPlayer.espCaixa) {
            Rect playerRect(boxX, boxY, width, boxHeight);
            if (pEspPlayer.boxType == BOX_FILLED || pEspPlayer.boxType == BOX_ROUNDED) {
                Color fill = colorBox; fill.a = 70;
                esp.DrawFilledRect(fill, Vector3(boxX, boxY, 0), width, boxHeight);
            }
            if (pEspPlayer.boxType == BOX_CORNER) esp.DrawBox4Line(boxX, boxY, width, boxHeight, colorBox, globalThickness);
            else esp.DrawBox(colorBox, globalThickness, playerRect);
        }

        //Draw Health
        if (pEspPlayer.espHealth) {
            float hBarW = (distance > 80.0f) ? 4.5f : 6.0f;
            float hBarX = boxX - (hBarW + 6);
            float hMax = 200.0f;
            esp.DrawVerticalHealthBar(Vector2(hBarX, boxY), boxHeight, hMax, player.Health, hBarW, false);

            if (player.Health > 0) {
                std::string healthText = std::to_string(player.Health) + "HP";
                float textSize = fSize * 0.7f;
                float healthPercent = (float)player.Health / hMax;
                if (healthPercent > 1.0f) healthPercent = 1.0f;
                float textY = boxY + (1.0f - healthPercent) * boxHeight + (textSize * 0.25f);
                float approxTextWidth = healthText.length() * (textSize * 0.40f);
                float textX = hBarX - approxTextWidth - 6;

                esp.DrawText(strokeColor, healthText.c_str(), Vector2(textX - 1, textY - 1), textSize);
                esp.DrawText(strokeColor, healthText.c_str(), Vector2(textX + 1, textY + 1), textSize);
                esp.DrawText(strokeColor, healthText.c_str(), Vector2(textX - 1, textY + 1), textSize);
                esp.DrawText(strokeColor, healthText.c_str(), Vector2(textX + 1, textY - 1), textSize);
                esp.DrawText(whiteColor, healthText.c_str(), Vector2(textX, textY), textSize);
            }
        }

        //Draw Player Name
        if (pEspPlayer.espName && player.PlayerName[0] != '\0') {
            float fSizeName = fSize;
            float nameY = boxY - fSizeName;

            if (player.Head.X  > 0 && player.Head.X < screenWidth) {
                esp.DrawText(strokeColor, player.PlayerName, Vector2(player.Head.X - 1, nameY - 1), fSizeName);
                esp.DrawText(strokeColor, player.PlayerName, Vector2(player.Head.X+ 1, nameY + 1), fSizeName);
                esp.DrawText(strokeColor, player.PlayerName, Vector2(player.Head.X - 1, nameY + 1), fSizeName);
                esp.DrawText(strokeColor, player.PlayerName, Vector2(player.Head.X + 1, nameY - 1), fSizeName);
                esp.DrawText(whiteColor, player.PlayerName, Vector2(player.Head.X, nameY), fSizeName);
            }
        }

        // Draw Distance
        float baseY = boxY + boxHeight + fSize + 8;
        float dY = baseY;
        if (pEspPlayer.espDistance && !player.IsCaido) {
            std::string dStr = std::to_string((int)distance) + "M";
            esp.DrawText(strokeColor, dStr.c_str(), Vector2(player.Head.X - 1, dY - 1), fSize);
            esp.DrawText(strokeColor, dStr.c_str(), Vector2(player.Head.X + 1, dY + 1), fSize);
            esp.DrawText(strokeColor, dStr.c_str(), Vector2(player.Head.X - 1, dY + 1), fSize);
            esp.DrawText(strokeColor, dStr.c_str(), Vector2(player.Head.X + 1, dY - 1), fSize);
            esp.DrawText(whiteColor, dStr.c_str(), Vector2(player.Head.X, dY), fSize);
        }

        // Draw Weapon
        if (pEspPlayer.espWeapon && player.WeaponId != 0) {
            std::string weaponName = Namegun::GetGunName(player.WeaponId);
            if (!weaponName.empty() && weaponName != "Unknown" && weaponName.substr(0, 3) != "ID:") {
                float weaponY = baseY + (pEspPlayer.espDistance ? fSize + 4 : 0);
                esp.DrawText(strokeColor, weaponName.c_str(), Vector2(player.Head.X - 1, weaponY - 1), fSize);
                esp.DrawText(strokeColor, weaponName.c_str(), Vector2(player.Head.X + 1, weaponY + 1), fSize);
                esp.DrawText(strokeColor, weaponName.c_str(), Vector2(player.Head.X - 1, weaponY + 1), fSize);
                esp.DrawText(strokeColor, weaponName.c_str(), Vector2(player.Head.X + 1, weaponY - 1), fSize);
                esp.DrawText(whiteColor, weaponName.c_str(), Vector2(player.Head.X, weaponY), fSize);
            }
        }
    }
}

void toggleShortcutInJava(JNIEnv *env, jclass clazz, int id, const char* name, int state) {
    jmethodID method = env->GetStaticMethodID(clazz, "manageShortcutButton", "(ILjava/lang/String;I)V");
    if (method) {
        jstring jname = env->NewStringUTF(name);
        env->CallStaticVoidMethod(clazz, method, id, jname, state);
        env->DeleteLocalRef(jname);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_LoginActivity_setLicenseSession(JNIEnv *env, jclass clazz, jboolean verified) {
    kbjisui8821 = verified ? 9251 : 0;
    uisid2iuwaw = verified ? 7821 : 8261;
    if (verified) Namegun::Init();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_Functions(JNIEnv *env, jclass clazz) {
    Widget widget = Widget(env);
    if (uisid2iuwaw == 8261) {
        widget.AddTab(OBFUSCATE("Open app to unlock"));
    }
    else if (uisid2iuwaw == 7821 ) {
        widget.AddTab(OBFUSCATE("Aimbot"));
        widget.Switch(OBFUSCATE("Enable Functions"), 1);
        widget.Switch(OBFUSCATE("Aim Collider"), 6);
        widget.Switch(OBFUSCATE("AimBot"), 28);
        widget.Switch(OBFUSCATE("Aim Silent"), 8);
        widget.Switch(OBFUSCATE("Aim Magnet Lite"), 37);
        widget.SeekBar(OBFUSCATE("AimFov"), 0, 180, OBFUSCATE(""), 9);
        widget.SeekBar(OBFUSCATE("Head Rate"), 0, 3, OBFUSCATE(""), 70);
        widget.SeekBar(OBFUSCATE("Chest Rate"), 0, 6, OBFUSCATE(""), 7);

        widget.AddTab(OBFUSCATE("Aim Helper"));
        /*widget.Switch(OBFUSCATE("AimLock"), 5);
        widget.Switch(OBFUSCATE("No Recoil"), 35);
        widget.Switch(OBFUSCATE("No Reload"), 34);*/
        //widget.Switch(OBFUSCATE("Aim Lag"), 2);
        widget.Switch(OBFUSCATE("Ignore Knock"), 22);
        widget.Switch(OBFUSCATE("Smart Kill"), 36);
        widget.Switch(OBFUSCATE("Draw AimFov"), 27);

        widget.AddTab(OBFUSCATE("Visuals"));
        widget.Switch(OBFUSCATE("Esp Line"), 11);
        widget.Switch(OBFUSCATE("Esp Box"), 12);
        widget.Switch(OBFUSCATE("Esp Distance"), 13);
        widget.Switch(OBFUSCATE("Esp Name"), 21);
        widget.Switch(OBFUSCATE("Esp Health"), 19);
        widget.Switch(OBFUSCATE("Esp Count"), 31);
        widget.Switch(OBFUSCATE("Esp Weapon"), 40);

        widget.Category(OBFUSCATE("Esp Config"));
        widget.SeekBar(OBFUSCATE("ESP Color"), 0, 9, OBFUSCATE("Color"), 16);
        widget.SeekBar(OBFUSCATE("Box Type"), 0, 3, OBFUSCATE("BoxType"), 17);
        widget.SeekBar(OBFUSCATE("Line Type"), 0, 2, OBFUSCATE("LineType"), 18);
        widget.SeekBar(OBFUSCATE("Text Size"), 15, 25, OBFUSCATE(""), 20);

        widget.AddTab(OBFUSCATE("Brutal"));
        /*widget.Switch(OBFUSCATE("High FPS"), 42);
        widget.Switch(OBFUSCATE("Tele Kill"), 23);*/
        //widget.Switch(OBFUSCATE("TeleKill Car"), 23);
        widget.Switch(OBFUSCATE("TP Mark"), 38);
        widget.Switch(OBFUSCATE("Fly Hack"), 4);
        widget.Switch(OBFUSCATE("Speed Hack"), 25);
        widget.Switch(OBFUSCATE("PC Movement"), 5);
        widget.Switch(OBFUSCATE("Dive Kill"), 3);
        widget.Switch(OBFUSCATE("Scale Down"), 26);
        widget.Switch(OBFUSCATE("Ghost Hack"), 33);
        widget.Switch(OBFUSCATE("Spin Bot"), 29);
        widget.Switch(OBFUSCATE("WallHack Bug"), 30);

        widget.AddTab(OBFUSCATE("Shortcuts"));
        widget.Switch(OBFUSCATE("Aim Magnet Shortcut"), 107);
        //widget.Switch(OBFUSCATE("Tele Kill Shortcut"), 100);
        widget.Switch(OBFUSCATE("TP Mark Shortcut"), 106);
        widget.Switch(OBFUSCATE("Fly Hack Shortcut"), 108);
        widget.Switch(OBFUSCATE("Speed Hack Shortcut"), 101);
        widget.Switch(OBFUSCATE("Dive Kill Shortcut"), 102);
        widget.Switch(OBFUSCATE("Scale Down Shortcut"), 103);
        widget.Switch(OBFUSCATE("Ghost Hack Shortcut"), 104);
        widget.Switch(OBFUSCATE("WallHack Bug Shortcut"), 105);
    }
}

std::string do_curl(const std::string& url) {
    CURL *curl = curl_easy_init();
    std::string result = "";
    if(curl) {
        MemoryChunk chunk;
        chunk.memory = (char *)malloc(1);
        chunk.size = 0;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            result = std::string(chunk.memory, chunk.size);
        }
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }
    return result;
}

void CppLog(JNIEnv *env, jclass clazz, const std::string& msg) {
    jmethodID logMethod = env->GetStaticMethodID(clazz, "addLogMsg", "(Ljava/lang/String;)V");
    if (logMethod != nullptr && !env->ExceptionCheck()) {
        jstring jMsg = env->NewStringUTF(msg.c_str());
        env->CallStaticVoidMethod(clazz, logMethod, jMsg);
        env->DeleteLocalRef(jMsg);
    } else {
        env->ExceptionClear();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_ChangesID(JNIEnv *env, jclass clazz, jint id, jint value) {
    // License verification now happens on the opening screen over HTTPS.
    if (id == 1000) return;

    if (uisid2iuwaw == 7821) {
        switch (id) {
            case 1:
                Actived.activar = !Actived.activar;
                SendBool(3, Actived.activar);
                break;
            /*case 2:
                pPlayer.AimLag = !pPlayer.AimLag;
                SendBool(71, pPlayer.AimLag);
                break;*/
            case 5:
                pPlayer.PCMovement = !pPlayer.PCMovement;
                SendBool(42, pPlayer.PCMovement);
                break;
            case 6:
                pPlayer.aimbotlock = !pPlayer.aimbotlock;
                SendBool(54, pPlayer.aimbotlock);
                break;
            case 7:
                pPlayer.ChestRate = value;
                SendFloat(62, pPlayer.ChestRate);
                break;
            case 8:
                pPlayer.silen4a = !pPlayer.silen4a;
                SendBool(57, pPlayer.silen4a);
                break;
            case 9:
                pPlayer.AimFov = value;
                SendFloat(58, pPlayer.AimFov);
                break;
            case 10:
                pPlayer.AimDistance = value;
                SendFloat(59, pPlayer.AimDistance);
                break;
            case 11:
                pEspPlayer.espLinha = !pEspPlayer.espLinha;
                break;
            case 12:
                pEspPlayer.espCaixa = !pEspPlayer.espCaixa;
                break;
            case 13:
                pEspPlayer.espDistance = !pEspPlayer.espDistance;
                break;
            case 14:
                pEspPlayer.espSkeleton = !pEspPlayer.espSkeleton;
                break;
            case 15:
                pEspPlayer.espIden360 = !pEspPlayer.espIden360;
                break;
            case 16:
                if (value == 0) pEspPlayer.espColor = Color(255, 255, 255, 255);
                else if (value == 1) pEspPlayer.espColor = Color(0, 255, 0, 255);
                else if (value == 2) pEspPlayer.espColor = Color(0, 0, 255, 255);
                else if (value == 3) pEspPlayer.espColor = Color(255, 0, 0, 255);
                else if (value == 4) pEspPlayer.espColor = Color(0, 0, 0, 255);
                else if (value == 5) pEspPlayer.espColor = Color(255, 255, 0, 255);
                else if (value == 6) pEspPlayer.espColor = Color(0, 255, 255, 255);
                else if (value == 7) pEspPlayer.espColor = Color(255, 0, 255, 255);
                else if (value == 8) pEspPlayer.espColor = Color(0, 80, 255, 255);
                else if (value == 9) pEspPlayer.espColor = Color(160, 32, 240, 255);
                break;
            case 17:
                pEspPlayer.boxType = static_cast<BoxType>(value);
                break;
            case 18:
                pEspPlayer.lineType = static_cast<LineType>(value);
                break;
            case 19:
                pEspPlayer.espHealth = !pEspPlayer.espHealth;
                break;
            case 20:
                pEspPlayer.textSize = (float) value;
                break;
            case 21:
                pEspPlayer.espName = !pEspPlayer.espName;
                break;
            /*case 23:
                pPlayer.TeleKill = !pPlayer.TeleKill;
                SendBool(46, pPlayer.TeleKill);
                break;*/
            case 40:
                pEspPlayer.espWeapon = !pEspPlayer.espWeapon;
                break;
            case 22:
                pPlayer.IgnoreKnock = !pPlayer.IgnoreKnock;
                SendBool(55, pPlayer.IgnoreKnock);
                break;
            case 27:
                pEspPlayer.drawFov = !pEspPlayer.drawFov;
                break;
            case 28:
                pPlayer.aimhard = !pPlayer.aimhard;
                SendBool(51, pPlayer.aimhard);
                break;
            case 29:
                pPlayer.SpinBot = !pPlayer.SpinBot;
                SendBool(50, pPlayer.SpinBot);
                break;
            case 31:
                pEspPlayer.espCount = !pEspPlayer.espCount;
                break;
            case 36:
                pPlayer.KillInChest = !pPlayer.KillInChest;
                SendBool(45, pPlayer.KillInChest);
                break;
            case 70:
                pPlayer.HeadRate = value;
                SendFloat(70, pPlayer.HeadRate);
                break;
            case 25:
                pPlayer.SpeedTimer = !pPlayer.SpeedTimer;
                SendBool(52, pPlayer.SpeedTimer);
                if (showSpeed) {
                    toggleShortcutInJava(env, clazz, 25, "Speed Hack", pPlayer.SpeedTimer ? 1 : 2);
                }
                break;
            case 101:
                showSpeed = !showSpeed;
                if (showSpeed) {
                    toggleShortcutInJava(env, clazz, 25, "Speed Hack", pPlayer.SpeedTimer ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 25, "Speed Hack", 0);
                }
                break;

            case 3:
                pPlayer.DiveKill = !pPlayer.DiveKill;
                SendBool(59, pPlayer.DiveKill);
                if (showDive) {
                    toggleShortcutInJava(env, clazz, 3, "Dive Kill", pPlayer.DiveKill ? 1 : 2);
                }
                break;
            case 102:
                showDive = !showDive;
                if (showDive) {
                    toggleShortcutInJava(env, clazz, 3, "Dive Kill", pPlayer.DiveKill ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 3, "Dive Kill", 0);
                }
                break;

            case 26:
                pPlayer.ScaleDown = !pPlayer.ScaleDown;
                SendBool(53, pPlayer.ScaleDown);
                if (showScale) {
                    toggleShortcutInJava(env, clazz, 26, "Scale Down", pPlayer.ScaleDown ? 1 : 2);
                }
                break;
            case 103:
                showScale = !showScale;
                if (showScale) {
                    toggleShortcutInJava(env, clazz, 26, "Scale Down", pPlayer.ScaleDown ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 26, "Scale Down", 0);
                }
                break;
            case 33:
                pPlayer.GhostHack = !pPlayer.GhostHack;
                SendBool(48, pPlayer.GhostHack);
                if (showGhost) {
                    toggleShortcutInJava(env, clazz, 33, "Ghost Hack", pPlayer.GhostHack ? 1 : 2);
                }
                break;
            case 104:
                showGhost = !showGhost;
                if (showGhost) {
                    toggleShortcutInJava(env, clazz, 33, "Ghost Hack", pPlayer.GhostHack ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 33, "Ghost Hack", 0);
                }
                break;

            case 30:
                pPlayer.WallBug = !pPlayer.WallBug;
                SendBool(49, pPlayer.WallBug);
                if (showWall) {
                    toggleShortcutInJava(env, clazz, 30, "WallHack Bug", pPlayer.WallBug ? 1 : 2);
                }
                break;
            case 105:
                showWall = !showWall;
                if (showWall) {
                    toggleShortcutInJava(env, clazz, 30, "WallHack Bug", pPlayer.WallBug ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 30, "WallHack Bug", 0);
                }
                break;

            case 38:
                pPlayer.TPMark = !pPlayer.TPMark;
                SendBool(43, pPlayer.TPMark);
                if (showMark) {
                    toggleShortcutInJava(env, clazz, 38, "TP Mark", pPlayer.TPMark ? 1 : 2);
                }
                break;
            case 106:
                showMark = !showMark;
                if (showMark) {
                    toggleShortcutInJava(env, clazz, 38, "TP Mark", pPlayer.TPMark ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 38, "TP Mark", 0);
                }
                break;
            case 37:
                pPlayer.pullenemy = !pPlayer.pullenemy;
                SendBool(44, pPlayer.pullenemy);
                if (showMagnet) {
                    toggleShortcutInJava(env, clazz, 37, "Aim Magnet", pPlayer.pullenemy ? 1 : 2);
                }
                break;
            case 107:
                showMagnet = !showMagnet;
                if (showMagnet) {
                    toggleShortcutInJava(env, clazz, 37, "Aim Magnet", pPlayer.pullenemy ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 37, "Aim Magnet", 0);
                }
                break;

            case 4:
                pPlayer.FlyHack = !pPlayer.FlyHack;
                SendBool(41, pPlayer.FlyHack);
                if (showFly) {
                    toggleShortcutInJava(env, clazz, 4, "Fly Hack", pPlayer.FlyHack ? 1 : 2);
                }
                break;
            case 108:
                showFly = !showFly;
                if (showFly) {
                    toggleShortcutInJava(env, clazz, 4, "Fly Hack", pPlayer.FlyHack ? 1 : 2);
                } else {
                    toggleShortcutInJava(env, clazz, 4, "Fly Hack", 0);
                }
                break;
        }
    }
}


extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_PxbftKZXivSr(JNIEnv *env, jclass type, jobject espView, jobject canvas, jint width, jint height) {
    ESP localEsp = ESP(env, espView, canvas);
    if (localEsp.isValid()) { DrawESP(localEsp); }
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_ftKZXivSr(JNIEnv *env, jclass clazz, jobject ctx) {
    if (!Connection.connectClient) {
        startClient();
        Connection.connectClient = true;
        //Toast(env, ctx, "ACTIVATING...", 1, 1);
        //Toast(env, ctx, "ACTIVATED", 1, 1);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_ftKwCvSr(JNIEnv *env, jclass clazz) {
    stopClient();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_microsoft_xboxone_smartglass_LoginActivity_getObfuscatedString(JNIEnv *env, jclass clazz, jint id) {
    switch(id) {
        case 0: return env->NewStringUTF(OBFUSCATE("BKL"));
        case 1: return env->NewStringUTF(OBFUSCATE(" SENSI"));
        case 2: return env->NewStringUTF(OBFUSCATE("DEVELOPER @babykingleo12"));
        case 3: return env->NewStringUTF(OBFUSCATE("Execute Hack"));
        case 4: return env->NewStringUTF(OBFUSCATE("SPECIAL THANKS TO CRACKED TEAM"));
        case 5: return env->NewStringUTF(OBFUSCATE("https://t.me/bklsensi"));
        case 6: return env->NewStringUTF(OBFUSCATE("https://discord.gg/nVf4uzRggS"));
        case 7: return env->NewStringUTF(OBFUSCATE("https://www.instagram.com/leo_soare09"));
        case 8: return env->NewStringUTF(OBFUSCATE("https://www.youtube.com/@BKLSENSI"));
        case 9: return env->NewStringUTF(OBFUSCATE("https://t.me/crackedteamglobal"));
        case 10: return env->NewStringUTF(OBFUSCATE("Select Root Method"));
        case 11: return env->NewStringUTF(OBFUSCATE("Root Selected : "));
        case 12: return env->NewStringUTF(OBFUSCATE("Magisk"));
        case 13: return env->NewStringUTF(OBFUSCATE("KernelSU / APatch"));
        case 14: return env->NewStringUTF(OBFUSCATE("Cloud Phones"));
        case 15: return env->NewStringUTF(OBFUSCATE("Executing…"));
        case 16: return env->NewStringUTF(OBFUSCATE("Select Game Version"));
        case 17: return env->NewStringUTF(OBFUSCATE("Want to donate for my work?"));
        case 18: return env->NewStringUTF(OBFUSCATE("https://www.paypal.me/babykingleo"));
        case 19: return env->NewStringUTF(OBFUSCATE("Kernel Driver"));
        default: return env->NewStringUTF("");
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_xboxone_smartglass_LoginActivity_getObfuscatedColor(JNIEnv *env, jclass clazz, jint id) {
    switch(id) {
        case 50: return (jint)0xFF0A0A0C;
        case 51: return (jint)0xFF0F0F12;
        case 52: return (jint)0xFF1C1C24;
        case 53: return (jint)0xFF0051FF;
        case 54: return (jint)0xFFFFFFFF;
        case 55: return (jint)0xFF62626E;
        case 56: return (jint)0xFF2A1414;
        case 57: return (jint)0xFF142A19;
        case 58: return (jint)0xFFE53935;
        case 59: return (jint)0xFF43A047;
        case 60: return (jint)0xFFFFCDD2;
        case 61: return (jint)0xFFC8E6C9;
        default: return (jint)0xFFFFFFFF;
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_getObfuscatedString(JNIEnv *env, jclass clazz, jint id) {
    switch(id) {
        case 100: return env->NewStringUTF(OBFUSCATE("STORM MODS"));
        case 101: return env->NewStringUTF(OBFUSCATE("Minimize"));
        case 102: return env->NewStringUTF(OBFUSCATE("Authentification"));
        case 103: return env->NewStringUTF(OBFUSCATE("Login Succesful!"));
        case 120: return env->NewStringUTF(OBFUSCATE("White"));
        case 121: return env->NewStringUTF(OBFUSCATE("Green"));
        case 122: return env->NewStringUTF(OBFUSCATE("Blue"));
        case 123: return env->NewStringUTF(OBFUSCATE("Red"));
        case 124: return env->NewStringUTF(OBFUSCATE("Black"));
        case 125: return env->NewStringUTF(OBFUSCATE("Yellow"));
        case 126: return env->NewStringUTF(OBFUSCATE("Cyan"));
        case 127: return env->NewStringUTF(OBFUSCATE("Magenta"));
        case 128: return env->NewStringUTF(OBFUSCATE("Azure"));
        case 129: return env->NewStringUTF(OBFUSCATE("Purple"));
        case 130: return env->NewStringUTF(OBFUSCATE("#ffffff"));
        case 131: return env->NewStringUTF(OBFUSCATE("#00FF00"));
        case 132: return env->NewStringUTF(OBFUSCATE("#0000FF"));
        case 133: return env->NewStringUTF(OBFUSCATE("#FF0000"));
        case 134: return env->NewStringUTF(OBFUSCATE("#000000"));
        case 135: return env->NewStringUTF(OBFUSCATE("#FFFF00"));
        case 136: return env->NewStringUTF(OBFUSCATE("#00FFFF"));
        case 137: return env->NewStringUTF(OBFUSCATE("#FF00FF"));
        case 138: return env->NewStringUTF(OBFUSCATE("#0050FF"));
        case 139: return env->NewStringUTF(OBFUSCATE("#A020F0"));
        case 140: return env->NewStringUTF(OBFUSCATE("Stroke"));
        case 141: return env->NewStringUTF(OBFUSCATE("Filled"));
        case 142: return env->NewStringUTF(OBFUSCATE("Corner"));
        case 150: return env->NewStringUTF(OBFUSCATE("Top"));
        case 151: return env->NewStringUTF(OBFUSCATE("Center"));
        case 152: return env->NewStringUTF(OBFUSCATE("Bottom"));
        default: return env->NewStringUTF("");
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_xboxone_smartglass_Floater_getObfuscatedColor(JNIEnv *env, jclass clazz, jint id) {
    switch(id) {
        case 200: return (jint)0xFF64E5C5;
        case 201: return (jint)0xF5090D16;
        case 202: return (jint)0x15FFFFFF;
        case 203: return (jint)0xFF9BAAC2;
        case 204: return (jint)0xFF243F43;
        case 205: return (jint)0xFF141C2C;
        case 206: return (jint)0x80FFFFFF;
        case 207: return (jint)0xFF141C2C;
        case 208: return (jint)0xFF1E2A3E;
        case 209: return (jint)0x99FFFFFF;
        case 210: return (jint)0xFF284C4D;
        case 211: return (jint)0xFF64E5C5;
        case 212: return (jint)0xFF64E5C5;
        case 213: return (jint)0xFFFF929B;
        case 214: return (jint)0xFFFFFFFF;
        case 215: return (jint)0xFF000000;
        case 216: return (jint)0x00000000;
        default: return (jint)0xFFFFFFFF;
    }
}