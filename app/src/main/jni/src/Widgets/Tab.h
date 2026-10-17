#ifndef TAB_H
#define TAB_H

#include <jni.h>

class Tab {
public:
    void create(JNIEnv* env, const char* name) {
        jclass CMenu = env->FindClass("com/microsoft/xboxone/smartglass/Floater");
        jmethodID MTab = env->GetStaticMethodID(CMenu, "addTab", "(Ljava/lang/String;)V");
        env->CallStaticVoidMethod(CMenu, MTab, env->NewStringUTF(name));
    }
};

#endif