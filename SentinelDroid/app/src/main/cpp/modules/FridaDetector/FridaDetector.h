//
// Created by taiyi on 2025/5/8.
//

#ifndef SENTINELDROID_FRIDADETECTOR_H
#define SENTINELDROID_FRIDADETECTOR_H

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
#include <vector>
#include <string>
#include <regex>
#include <arpa/inet.h>
#include <dlfcn.h>

class fridaDetector {
public:
  //检测frida安装目录(frida-server)
  bool Detect_FeaturesFile();
 //检查27042该端口是否被占用的方式来检测frida
  bool Detect_NetworkPort();
  //检查当前运行的进程是否有frida相关的进程在运行。
  bool Detect_processName();
  //检测线程相关
  bool Detect_Tasks();
  //检测D-Bus通信特征
  bool Detect_DBus();
  //检测已加载的 Frida 动态库
  bool Detect_loadLib();

  //检查内存特征
  bool Detect_memory();

  //检测dlopen或dlsym
  bool Detect_runtimeLibrary();

  //inlineHook检测
  bool Detect_inlineHook();


};


#endif //SENTINELDROID_FRIDADETECTOR_H
