class Button {
public:
    void create(JNIEnv* env, const char* name, jint ID) {
        jclass CMenu = env->FindClass("com/microsoft/xboxone/smartglass/Floater");
        jmethodID MBtn = env->GetStaticMethodID(CMenu, "addButton", "(Ljava/lang/String;I)V");
        env->CallStaticVoidMethod(CMenu, MBtn, env->NewStringUTF(name), ID);
    }
};