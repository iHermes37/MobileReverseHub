#ifndef RETOOLKIT_LIBCHOOK_H
#define RETOOLKIT_LIBCHOOK_H


#include <stddef.h>
#include <stdint.h>
#include <android/log.h>
#include <bytehook.h>

class libcHook {

//hook具体操作函数
public:

    void hook_memcpy();

//   libcHook(){
//       bytehook_init();
//   }


// Proxy 函数
public:
    static void *proxy_memcpy(void *dest, const void *src, size_t n);


//原始定义PLT导出的函数定义
private:
    static void *(*orig_memcpy)(void *, const void *, size_t);

//工具变量
private:
    static volatile uint64_t g_memcpy_count;
};


#endif //RETOOLKIT_LIBCHOOK_H
