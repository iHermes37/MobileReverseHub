#include <android/log.h>
#include <jni.h>
#include "utils/libcHook.h"


extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    __android_log_print(ANDROID_LOG_ERROR, "JNIDebug", "✅ JNI_OnLoad called, success!");
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "JNIDebug", "Failed to get JNIEnv");
        return JNI_ERR;
    }
    __android_log_print(ANDROID_LOG_INFO, "JNIDebug", ">>> JNI_OnLoad ENTERED <<<");

    libcHook libc;
    libc.hook_memcpy();

    return JNI_VERSION_1_6;
}




