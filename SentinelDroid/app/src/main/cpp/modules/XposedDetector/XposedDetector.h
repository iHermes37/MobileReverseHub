//
// Created by taiyi on 2025/5/8.
//

#ifndef SENTINELDROID_XPOSEDDETECTOR_H
#define SENTINELDROID_XPOSEDDETECTOR_H
#include <jni.h>

class xposedDetector {
public:
    bool Detect_featuresFile();

    bool Tryload_XposedClass(JNIEnv *env);

    bool Detect_MethodCallStack();

    bool Detect_installList(JNIEnv *env, jobject thiz, jobject context);

//    bool Detect_nativeFunc();
//    bool Detect_map();
//    bool Detect_xposedProp();
//    bool Detect_xposedHelperMap();
//    bool Detect_featuresBinary();

};


#endif //SENTINELDROID_XPOSEDDETECTOR_H
