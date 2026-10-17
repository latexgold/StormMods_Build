//
// Created by Thiago on 31/05/2025.
//

#include <jni.h>
#include <sstream>
#include <string>

std::string concatenateStrings(const char* str1, const char* str2) {
    std::string result(str1);
    result += str2;
    return result.c_str();
}

void Toast(JNIEnv *env, jobject thiz, const char *text, int length,int Tipo ) {

    jstring jstr;
    jclass toastClass;
    jmethodID methodMakeText;
    jobject toastobj;
    jmethodID methodShow;

    jclass htmlClass;
    jmethodID methodFromHtml;
    jobject spannedStr;

    switch (Tipo) {
        case 1:
            jstr = env->NewStringUTF(concatenateStrings("<font color='green'>" , text).c_str());
            htmlClass = env->FindClass("android/text/Html");
            methodFromHtml = env->GetStaticMethodID( htmlClass, "fromHtml", "(Ljava/lang/String;)Landroid/text/Spanned;" );
            spannedStr = env->CallStaticObjectMethod(htmlClass, methodFromHtml, jstr);
            toastClass = env->FindClass(("android/widget/Toast"));
            methodMakeText =env->GetStaticMethodID(toastClass, ("makeText"), ("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
            toastobj = env->CallStaticObjectMethod(toastClass, methodMakeText, thiz, spannedStr, length);
            methodShow = env->GetMethodID(toastClass, ("show"), ("()V"));
            env->CallVoidMethod(toastobj, methodShow);
            break;

        case 2:
            jstr = env->NewStringUTF(concatenateStrings("<font color='red'>" , text).c_str());
            htmlClass = env->FindClass("android/text/Html");
            methodFromHtml = env->GetStaticMethodID( htmlClass, "fromHtml", "(Ljava/lang/String;)Landroid/text/Spanned;" );
            spannedStr = env->CallStaticObjectMethod(htmlClass, methodFromHtml, jstr);
            toastClass = env->FindClass(("android/widget/Toast"));
            methodMakeText =env->GetStaticMethodID(toastClass, ("makeText"), ("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
            toastobj = env->CallStaticObjectMethod(toastClass, methodMakeText, thiz, spannedStr, length);
            methodShow = env->GetMethodID(toastClass, ("show"), ("()V"));
            env->CallVoidMethod(toastobj, methodShow);
            break;

        case 3:
            jstr = env->NewStringUTF(text);
            toastClass = env->FindClass(("android/widget/Toast"));
            methodMakeText =env->GetStaticMethodID(toastClass, ("makeText"), ("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
            toastobj = env->CallStaticObjectMethod(toastClass, methodMakeText, thiz, jstr, length);
            methodShow = env->GetMethodID(toastClass, ("show"), ("()V"));
            env->CallVoidMethod(toastobj, methodShow);
            break;
    }
}

void Funcoes(JNIEnv *env, jobject ctx, const char *pkg, const char *libname, const char *offset, const char *hex){
    jclass Main = env->GetObjectClass(ctx);
    jmethodID AddFuncoes = env->GetMethodID(Main,("Funçoes"),("(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"));
    env->CallVoidMethod(ctx, AddFuncoes, env->NewStringUTF(pkg), env->NewStringUTF(libname), env->NewStringUTF(offset), env->NewStringUTF(hex));
}
