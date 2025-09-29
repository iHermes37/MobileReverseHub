// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("extractor");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("extractor")
//      }
//    }

#include <jni.h>
#include <dlfcn.h>
#include <bytehook.h>

// 原始 defineClass 函数指针
static jclass (*original_defineClass)(JNIEnv*, jclass, jstring, jbyteArray, jint, jint) = nullptr;

// Hook 后的 defineClass
jclass my_defineClass(JNIEnv* env, jclass clazz, jstring name, jbyteArray data, jint offset, jint length) {
    // 在这里添加你的 Hook 逻辑
    // 例如打印类名
    const char* className = env->GetStringUTFChars(name, nullptr);
    __android_log_print(ANDROID_LOG_INFO, "MyHook", "Loading class: %s", className);
    env->ReleaseStringUTFChars(name, className);

    // 调用原始函数
    return original_defineClass(env, clazz, name, data, offset, length);
}

// 注册 Hook
extern "C" JNIEXPORT void JNICALL
Java_com_example_MyApp_initHook(JNIEnv* env, jobject thiz) {
// 获取 ClassLoader 的 defineClass 方法
void* handle = dlopen("libart.so", RTLD_NOW);
void* sym = dlsym(handle, "_ZN3art11ClassLoader11DefineClassEP7_JNIEnvPKcmNS_6HandleINS_6mirror11ClassLoaderEEEPKhi");

// 安装 Hook
bytehook_stub_t stub = bytehook_hook_single(
        "libart.so",
        nullptr,
        sym,
        (void*)my_defineClass,
        (void**)&original_defineClass,
        nullptr);

if(stub == nullptr) {
__android_log_print(ANDROID_LOG_ERROR, "MyHook", "Hook failed");
} else {
__android_log_print(ANDROID_LOG_INFO, "MyHook", "Hook success");
}
}