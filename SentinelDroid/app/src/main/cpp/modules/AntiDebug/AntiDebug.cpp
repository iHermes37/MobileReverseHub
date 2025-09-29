//
// Created by taiyi on 2025/5/8.
//

#include "AntiDebug.h"

bool antiDebug::Detect_TracerPidStatus(){
    pid_t curPid=getpid();
    std::string statusPath="/proc/"+std::to_string(curPid)+"/status";
    while(1){
        std::ifstream statusFile(statusPath);
        if(!statusFile.is_open()){
            perror("无法打开状态文件");
            return true;
        }

        std::string line;
        while(std::getline(statusFile,line)){
            if(line.find("TracerPid:")==0){
                int tracerPid = 0;
                if (sscanf(line.c_str(), "TracerPid: %d", &tracerPid) == 1) {
                    if (tracerPid != 0) {
                        //std::cout << "进程正在被调试 (TracerPid = " << tracerPid << ")" << std::endl;
                        return false;
                    }
                }
                break; // 找到 TracerPid 后退出循环
            }
        }
        sleep(1);
    }
}
bool antiDebug::Detect_rtld_db_dlactivity() {
    std::ifstream mapsFile("/proc/self/maps");
    if (!mapsFile.is_open()) {
        return false;  // 文件打开失败
    }
    if(mapsFile.is_open()){
        std::string line;
        while(std::getline(mapsFile,line)){
            if(line.find("rtld_db")!=std::string::npos ||
               line.find("libdl")!=std::string::npos
                    ){
                mapsFile.close();
                return true;
            }
        }
        mapsFile.close();
    }
    return false;
}

bool antiDebug::Detect_isDebuggerConnected(JNIEnv* env) {
    jclass debugClass = env->FindClass("android/os/Debug");
    if (debugClass == nullptr) {
        return false;
    }

    jmethodID isDebuggerConnected = env->GetStaticMethodID(
            debugClass,
            "isDebuggerConnected",
            "()Z"
    );

    if (isDebuggerConnected == nullptr) {
        env->DeleteLocalRef(debugClass);
        return false;
    }

    jboolean result = env->CallStaticBooleanMethod(debugClass, isDebuggerConnected);
    env->DeleteLocalRef(debugClass);
    return (bool)result;
}

void antiDebug::ptraceSelf(){
    // 尝试声明自身被跟踪（占坑）
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
        // 如果失败，说明已有调试器附加
        printf("检测到调试器！\n");
        _exit(1);
    }
    // 成功占坑后，其他调试器无法附加
    printf("反调试生效：当前进程已占用 ptrace 槽位\n");

    // 注意：此处不能调用 PTRACE_ATTACH，否则会解除 TRACEME 状态！
}

bool antiDebug::Detect_processName() {
    const char* path="/proc";
    DIR* dir= opendir(path);
    if (!dir) {
        perror("无法打开 /proc 目录");
        return true;
    }
    struct dirent* entry;
    while((entry= readdir(dir))!= nullptr){
        if(entry->d_type==DT_DIR && isdigit(entry->d_name[0])){
            pid_t pid= atoi(entry->d_name);
            std::string CommPath="/proc/"+std::string(entry->d_name)+"/comm";
            std::ifstream CommFile(CommPath);
            std::string ProcessName;
            if (!CommFile.is_open()) {
                continue;  // 跳过无法打开的文件
            }
            std::getline(CommFile,ProcessName);
            if(ProcessName.find("gdbserver")!=std::string::npos||
               ProcessName.find("android_server")!=std::string::npos
            ) {
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}
bool antiDebug::Detect_Port() {
    int port=23946;
    int sock= socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket 创建失败");
        return false;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // 设置 SO_REUSEADDR 避免 TIME_WAIT 状态干扰
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 尝试绑定
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return true;  // 绑定失败，说明端口被占用
    }

    close(sock);
    return false;

}
bool antiDebug::Detect_systemFiles() {
    const char* dirpath="/data/local/tmp";
    DIR* dir= opendir(dirpath);
    if(!dir){
        perror("opendir /data/local/tmp 失败");  // 输出错误信息
        return false;
    }
    struct dirent* entry;
    while((entry= readdir(dir))!= nullptr){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // 直接使用 strstr 避免不必要的 string 构造
        if (strstr(entry->d_name, "android_server") != nullptr) {
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}

bool antiDebug::Detect_breakPoint(){
    Dl_info info;
    if (!dladdr((void*)Detect_breakPoint, &info)) return false;
    ElfW(Phdr)* phdr = nullptr;
    ElfW(Half) phnum = 0;
    ElfW(Addr) base = 0;

    // 获取 ELF 头信息
    if (info.dli_fbase) {
        ElfW(Ehdr)* ehdr = (ElfW(Ehdr)*)info.dli_fbase;
        phdr = (ElfW(Phdr)*)((char*)ehdr + ehdr->e_phoff);
        phnum = ehdr->e_phnum;
        base = (ElfW(Addr))info.dli_fbase;
    }

    // 遍历程序头，查找可执行段
    for (ElfW(Half) i = 0; i < phnum; i++) {
        if (phdr[i].p_type != PT_LOAD || !(phdr[i].p_flags & PF_X)) continue;

        ElfW(Addr) seg_start = base + phdr[i].p_vaddr;
        ElfW(Addr) seg_end = seg_start + phdr[i].p_memsz;

        // 扫描内存
        for (ElfW(Addr) addr = seg_start; addr < seg_end - 3; addr++) {
            uint8_t* p = (uint8_t*)addr;
            if (*(uint32_t*)p == 0xef9f0001) return true;  // ARM
            if (*(uint16_t*)p == 0xde01) return true;      // Thumb16
            if (addr + 3 < seg_end && *(uint32_t*)p == 0xa000f7f0) return true; // Thumb32
        }
    }

    return false;
}




