#include "libcHook.h"

void* (*libcHook::orig_memcpy)(void*, const void*, size_t) = nullptr;
volatile uint64_t libcHook::g_memcpy_count = 0;


void* libcHook::proxy_memcpy(void *dest, const void *src, size_t n) {
    __android_log_print(ANDROID_LOG_DEBUG, "libcHook", "memcpy called: dest=%p, src=%p, n=%zu", dest, src, n);
    // 原子递增计数器
    __sync_fetch_and_add(&g_memcpy_count, 1);
    // 调用原函数
    return orig_memcpy(dest, src, n);
}

void libcHook::hook_memcpy() {
//    bytehook_stub_t stub = bytehook_hook_all(
//            "libc.so",       // 目标库（libc.so）
//            "memcpy",        // 函数名
//            (void *)proxy_memcpy, // 替换函数
//            nullptr,         // Hook 回调（可选）
//            nullptr          // 回调参数（可选）
//    );

    const char* symbols[] = {
            "__memcpy_chk",
            "__aeabi_memcpy_impl",
            "__memcpy_a53",
            "__memcpy_a55"
    };

    for (const char* sym : symbols) {
        bytehook_hook_single("libc.so", nullptr, sym, (void*)proxy_memcpy, nullptr, nullptr);
    }

    __android_log_print(ANDROID_LOG_INFO, "libcHook", "memcpy called count: %llu", g_memcpy_count);


}