
cmake_minimum_required(VERSION 3.22.1)


project("retoolkit")

find_package(bytehook REQUIRED CONFIG)
find_package(shadowhook REQUIRED CONFIG)

# SandHook 配置 - 根据架构设置源文件
#if(${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
#    set(OS_DEPENDENDED_SRC
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/assembler/assembler_arm64.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/inst/inst_arm64.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/register/register_arm64.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/register/register_list_arm64.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/decoder/decoder_arm64.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/relocate/code_relocate_arm64.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/hook/hook_arm64.cpp)
#
#elseif (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "armeabi-v7a")
#    set(OS_DEPENDENDED_SRC
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/inst/inst_arm32.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/inst/inst_t32.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/inst/inst_t16.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/register/register_arm32.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/register/register_list_arm32.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/assembler/assembler_arm32.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/decoder/decoder_arm32.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/hook/hook_arm32.cpp
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/hook/breakpoint_shellcode.S
#            ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/relocate/code_relocate_arm32.cpp)
#endif()


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)

# 主库配置
add_library(${CMAKE_PROJECT_NAME} SHARED
        retoolkit.cpp
        utils/libcHook.cpp
#        anti2dbg.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/sandhook_native.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/decoder/decoder.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/relocate/code_relocate.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/elf/elf.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/assembler/assembler.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/buffer/code_buffer.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/utils/platform.cpp
#        ${CMAKE_SOURCE_DIR}/sandhook/hook/hook.cpp
#        ${OS_DEPENDENDED_SRC}
)



#add_library(DobbyLib STATIC IMPORTED)
#set(DOBBY_LIB_DIR ${CMAKE_SOURCE_DIR}/../../../libs/arm64)
#set_target_properties(DobbyLib PROPERTIES
#        IMPORTED_LOCATION ${DOBBY_LIB_DIR}/libdobby.a
#)



target_link_libraries(${CMAKE_PROJECT_NAME}
#        DobbyLib
        bytehook::bytehook
#        shadowhook::shadowhook
        android log )

# 包含目录
#target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
#        ${CMAKE_SOURCE_DIR}/sandhook
#        ${CMAKE_SOURCE_DIR}/sandhook/asm
#        ${CMAKE_SOURCE_DIR}/sandhook/decoder
#        ${CMAKE_SOURCE_DIR}/sandhook/elf
#        ${CMAKE_SOURCE_DIR}/sandhook/utils
#        ${CMAKE_SOURCE_DIR}/sandhook/includes
#        ${CMAKE_SOURCE_DIR}/sandhook/buffer
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/inst
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/register
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/decoder
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/assembler
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/relocate
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm64/hook
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/inst
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/register
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/assembler
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/decoder
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/hook
#        ${CMAKE_SOURCE_DIR}/sandhook/archs/arm/arm32/relocate
#        ${CMAKE_SOURCE_DIR}/sandhook/antihook
#)
