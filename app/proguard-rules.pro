# 1. More aggressive renaming
-optimizationpasses 5
-allowaccessmodification
-overloadaggressively
-repackageclasses ''

# 2. Maximum Metadata Stripping
# This removes lines numbers, source files, and generic signatures
-keepattributes !*Annotation*,!Signature,!InnerClasses,!EnclosingMethod,!SourceFile,!LineNumberTable

# 3. Kill the Logs
# This is vital. It physically removes Log.d/i/v calls from the dex
-assumenosideeffects class android.util.Log {
    public static *** d(...);
    public static *** v(...);
    public static *** i(...);
    public static *** w(...);
    public static *** e(...);
}

# 4. Protect your MainActivity but NOT its contents
-keep public class com.microsoft.xboxone.smartglass.MainActivity
-keep public class com.microsoft.xboxone.smartglass.LoginActivity { *; }

# 5. JNI Protection (Necessary since you use NDK)
-keepclasseswithmembernames class * {
    native <methods>;
}
-keepclassmembers class com.microsoft.xboxone.Protector {
    public static java.lang.String getUrl();
    public static java.lang.String getOwner();
    public static java.lang.String getName();
    public static java.lang.String getVer();
}

# 6. libsu (Required to keep root functions working)
-keep class com.topjohnwu.libsu.** { *; }

# Java members referenced by native UI callbacks must retain their names.
-keep class com.microsoft.xboxone.smartglass.Floater { *; }
-keep class com.microsoft.xboxone.smartglass.ESPView { *; }
