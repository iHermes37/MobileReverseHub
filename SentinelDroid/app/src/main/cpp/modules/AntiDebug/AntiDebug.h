//
// Created by taiyi on 2025/5/8.
//

#ifndef SENTINELDROID_ANTIDEBUG_H
#define SENTINELDROID_ANTIDEBUG_H

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <fstream>
#include <sys/ptrace.h>
#include <jni.h>
#include <dirent.h>
#include <linux/in.h>
#include <sys/endian.h>
#include <sys/socket.h>
#include <dlfcn.h>
#include <link.h>


class antiDebug {
public:
private:
    //调试状态检测TracerPid
   bool Detect_TracerPidStatus();
   //java层调试函数检测[补充：Native 层检测（兼容 ART/Dalvik）]
   bool Detect_isDebuggerConnected(JNIEnv* env);

   bool Detect_Port();
   bool Detect_processName();
   bool Detect_systemFiles();

   //bool Detect_ExecutTimediff();

   //打开当前进程的内存映射文件,检查是否包含 rtld_db（动态链接器调试库）或 libdl（动态链接库）。
   bool Detect_rtld_db_dlactivity();

   static bool Detect_breakPoint();

   //ptrace自身附加
   void ptraceSelf();

   //void Multiprocess_debugging();
};


#endif //SENTINELDROID_ANTIDEBUG_H
