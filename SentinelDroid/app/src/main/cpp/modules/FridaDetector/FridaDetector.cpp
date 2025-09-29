//
// Created by taiyi on 2025/5/8.
//

#include "FridaDetector.h"

bool fridaDetector::Detect_FeaturesFile() {
  const char* dirpath="/data/local/tmp";
  DIR* dir= opendir(dirpath);
  if(!dir){
      perror("opendir /data/local/tmp 失败");  // 输出错误信息
      return false;
  }
  struct dirent* document;
  std::regex pattern(R"(frida)",std::regex::icase);
  while((document= readdir(dir))!= nullptr){
      char* DocName=document->d_name;
      std::string str(DocName);
      if(std::regex_search(str,pattern)){
          closedir(dir);
          return true;
      }
  }
  closedir(dir);
  return false;
}

bool fridaDetector::Detect_processName() {
    const char* path="/proc";
    DIR* dir= opendir(path);
    if (!dir) {
        perror("无法打开 /proc 目录");
        return true;
    }
    struct dirent* entry;
    std::regex pattern(R"(frida)",std::regex::icase);
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
            if(std::regex_search(ProcessName,pattern)) {
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}

bool fridaDetector::Detect_NetworkPort() {
   int port=27042;
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

bool fridaDetector::Detect_Tasks(){
    const char* selfTaskPath="/proc/self/task";
    DIR* selfTaskDir= opendir(selfTaskPath);
    std::vector<std::string> tids;
    if(selfTaskDir){
        printf("error");
        return false;
    }
    struct dirent* doc;
    while((doc= readdir(selfTaskDir))!= nullptr){
        if(doc->d_type == DT_DIR && isdigit(doc->d_name[0])){
            tids.push_back(doc->d_name);
        }
    }
    closedir(selfTaskDir);

    for(int i=0;i<tids.size();i++){
        std::string commPath="/proc/self/task/"+tids[i]+"/comm";
        std::fstream commFile(commPath);
        if(commFile.is_open()){
            std::string taskName;
            std::getline(commFile,taskName);
            commFile.close();

            if(taskName.find("gmain")!=std::string::npos ||
               taskName.find("gdbus")!=std::string::npos ||
               taskName.find("gum-js-loop")!=std::string::npos ||
               taskName.find("pool-frida")!=std::string::npos
            ){
                return true;
            }
        }
    }

    return false;
}

bool fridaDetector::Detect_DBus(){
    int sock;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family=AF_INET;//设置IPv4
    inet_aton("127.0.0.1", &sa.sin_addr);//将 IP 地址字符串 "127.0.0.1" 转换为网络字节序的二进制形式, 并将其存储在 sa.sin_addr 中
    for(int i=0;i<=65535;++i){
        sock = socket(AF_INET, SOCK_STREAM, 0);//设置套接字
        sa.sin_port=htons(i);//设置端口号， htons(i) 将主机字节序转换为网络字节序
        //使用 connect 函数连接到指定的 IP 地址和端口号
        if(connect(sock,(struct sockaddr*)(&sa),sizeof(sa))!= -1){

            char res[7] = {0};
            if (send(sock, "\x00AUTH\r\n", 7, 0) == -1) {  // 合并发送
                close(sock);
                return false;
            }
            if (recv(sock, res, 6, 0) > 0 && strcmp(res, "REJECT") == 0) {
                close(sock);
                return true;  // 检测到 frida-server
            }
        }
    }
    close(sock);
    return false;
}

bool fridaDetector::Detect_loadLib() {
    std::ifstream mapsFile("/proc/self/maps");
    if(mapsFile.is_open()){
        std::string line;
        while(std::getline(mapsFile,line)){
            if (line.find("frida") != std::string::npos) {
                mapsFile.close();
                return true;
            }
        }
        mapsFile.close();
    }
    return false;
}


bool fridaDetector::Detect_memory(){
    std::string targetName = "LIBFRIDA";
    std::ifstream mapsFile("/proc/self/maps");
    char permission[512];
    unsigned long start, end;
    if(mapsFile.is_open()) {
        std::string line;
        while (std::getline(mapsFile, line)) {
            sscanf(line.c_str(), "%lx-%lx %s", &start, &end, permission);
            if (permission[2] == 'x') {
                size_t regionSize = end - start;
                char *buffer = new char[regionSize];
                std::ifstream memFile("/proc/self/mem");
                if (memFile.is_open()) {
                    memFile.seekg(start);
                    memFile.read(buffer, regionSize);
                    if (std::search(
                            buffer,                  // 内存起始地址
                            buffer + regionSize,     // 内存结束地址
                            targetName.begin(),      // 目标字符串起始迭代器
                            targetName.end()         // 目标字符串结束迭代器
                    ) != buffer + regionSize) {
                        delete[] buffer;
                        memFile.close();
                        mapsFile.close();
                        return true;
                    }
                    memFile.close();
                }
                delete[] buffer;
            }
        }
        mapsFile.close();
    }
    return false;
}

bool fridaDetector::Detect_runtimeLibrary(){
    // 1. 检查常见库名
    const char* frida_libs[] = {"libfrida-agent.so", "libfrida-gadget.so"};
    for (int i = 0; i < 2; i++) {
        if (dlopen(frida_libs[i], RTLD_NOW | RTLD_NOLOAD)) {
            return true;
        }
    }
    // 2. 检查关键符号
    if (dlsym(RTLD_DEFAULT, "frida_agent_main") ||
        dlsym(RTLD_DEFAULT, "gum_interceptor_attach")) {
        return true;
    }
    return false;
}

bool fridaDetector::Detect_inlineHook(){

}