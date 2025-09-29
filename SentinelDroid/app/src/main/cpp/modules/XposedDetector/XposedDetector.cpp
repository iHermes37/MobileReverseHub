//
// Created by taiyi on 2025/5/8.
//

#include "XposedDetector.h"

bool xposedDetector::Detect_installList(JNIEnv *env,jobject thiz, jobject context){
    //获取 Context 类信息
    jclass contextClass=env->GetObjectClass(context);
    //获取 getPackageManager 方法 ID
    jmethodID getPackageManagerMethod = env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");

    //获取 PackageManager 类和方法
    jclass packageManagerClass = env->FindClass("android/content/pm/PackageManager");
    jmethodID getPackageInfoMethod = env->GetMethodID(packageManagerClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");

    // 获取 PackageManager 实例
    jobject packageManager = env->CallObjectMethod(context, getPackageManagerMethod);
    //创建要检测的包名字符串
    jstring packageName = env->NewStringUTF("de.robv.android.xposed.installer");

    // 尝试获取包信息
    jobject packageInfo = nullptr;
    try {
         packageInfo = env->CallObjectMethod(packageManager, getPackageInfoMethod, packageName, 1); // GET_ACTIVITIES = 1
    } catch (...) {
        if (env->ExceptionCheck()) {
            jthrowable exception = env->ExceptionOccurred();
            jclass nameNotFoundExceptionClass = env->FindClass("android/content/pm/PackageManager$NameNotFoundException");
            if (env->IsInstanceOf(exception, nameNotFoundExceptionClass)) {
                                env->ExceptionClear();
                                return JNI_FALSE;
                            }
                            env->ExceptionClear();
                        }
                        return JNI_FALSE;
                }

    if (packageInfo != nullptr) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}


bool xposedDetector::Detect_featuresFile() {
    std::ifstream mapsFile("/proc/self/maps");
    if(mapsFile.is_open()){
        std::string line;
        while(std::getline(mapsFile,line)){
            if (line.find("xposed") != std::string::npos)
            {
                mapsFile.close();
                return true;
            }
    }
         mapsFile.close();
    }
    return false;
}

bool xposedDetector::Tryload_XposedClass(JNIEnv *env){
    const char* xposedClasses[] = {
    "de/robv/android/xposed/XposedBridge",
    "de/robv/android/xposed/XposedHelpers",
    "de/robv/android/xposed/services/BaseService"
    };

    for (const char* cls : xposedClasses) {
        jclass clazz = env->FindClass(cls);
        if (clazz != nullptr) {
            env->DeleteLocalRef(clazz);
            return JNI_TRUE;
        }
        env->ExceptionClear();
    }
    return JNI_FALSE;
}

bool xposedDetector::Detect_MethodCallStack(){
    JNIEnv* env = GetEnv();
                if(env == NULL || mExceptionGlobalRef == 0 || mStackElementRef == 0)
                return false;

    jmethodID  throwable_init = env->GetMethodID(mExceptionGlobalRef, "<init>", "(Ljava/lang/String;)V");
    jobject throwable_obj = env->NewObject(mExceptionGlobalRef, throwable_init, env->NewStringUTF("test"));

    jmethodID throwable_getStackTrace = env->GetMethodID(mExceptionGlobalRef, "getStackTrace", "()[Ljava/lang/StackTraceElement;");
    jobjectArray jStackElements = (jobjectArray)env->CallObjectMethod(throwable_obj, throwable_getStackTrace);

    jmethodID jMthGetClassName = env->GetMethodID(mStackElementRef, "getClassName", "()Ljava/lang/String;");
    int len = env->GetArrayLength(jStackElements);
    LOG_PRINT_E("jStackElements = %p, jMthGetClassName = %p, len = %d", jStackElements, jMthGetClassName, len);

    for(int i = 0; i < len; i++){
        jobject jStackElement = env->GetObjectArrayElement(jStackElements, i);
        jstring jClassName = (jstring)env->CallObjectMethod(jStackElement, jMthGetClassName);
        const char* szClassName = env->GetStringUTFChars(jClassName, 0);
        LOG_PRINT_I("szClassName = %s", szClassName);
    }

    return true;
}