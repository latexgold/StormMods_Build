class TextBox {
public:
    void create(JNIEnv* env, const char* hint, jint ID) {
        jclass CMenu = env->FindClass("com/microsoft/xboxone/smartglass/Floater");
        jmethodID MText = env->GetStaticMethodID(CMenu, "addTextBox", "(Ljava/lang/String;I)V");
        env->CallStaticVoidMethod(CMenu, MText, env->NewStringUTF(hint), ID);
    }
};