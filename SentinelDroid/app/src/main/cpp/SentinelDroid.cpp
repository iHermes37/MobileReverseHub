#include <jni.h>
#include <string>

extern "C" JNIEXPORT void JNICALL
Java_com_anti_sentineldroid_AntiDebug_setAntiDebugCallback(
            JNIEnv* env,
    jclass type, jobject jCallback) {
    jclass jclazz = env->GetObjectClass(jCallback);
}
