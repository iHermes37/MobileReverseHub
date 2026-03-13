#pragma once

#include <cstdint>

using namespace std;

using Elf32_Addr = uint32_t;
using Elf32_Off = uint32_t;
using Elf32_Half = uint16_t;
using Elf32_Word = uint32_t;
using Elf32_Sword = int32_t;

using Elf64_Addr = uint64_t;
using Elf64_Off = uint64_t;
using Elf64_Half = uint16_t;
using Elf64_Word = uint32_t;
using Elf64_Sword = int32_t;
using Elf64_Xword = uint64_t;
using Elf64_Sxword = int64_t;

#define EI_NIDENT	16

typedef struct elf64_hdr {
    unsigned char e_indet[EI_NIDENT];//ELF文件的描述，是一个16字节的标识，表明当前文件的数据格式，位数等[16B]
    Elf64_Half e_type;//文件的标识字段标识文件的类型；比如可执行文件，共享目标文件[2B]
    Elf64_Half e_machine;//目标文件的体系结构(处理器架构)[2B]
    Elf64_Half e_version;//当前文件的版本[2B]

    Elf64_Addr e_entry;//程序的虚拟入口地址[8B]
    Elf64_Off  e_phoff;//程序头表的偏移地址[8B]
    Elf64_Off  e_shoff;//段表/节表的偏移[8B]

    Elf64_Word e_flags; //处理器相关的标志位[4B]
    Elf64_Half e_ehsize; //ELF文件头的大小[2B]

    Elf64_Half e_phentsize;//程序头表的单项大小[2B]
    Elf64_Half e_phnum;//程序头表的单项数目[2B]

    Elf64_Half e_shentsize;//节表中单项的大小[2B]
    Elf64_Half e_shnum;//节表中单项的数目[2B]
    Elf64_Half e_shstrndx;//节表中节名的索引[2B]------- 存储节名称，它包含所有节头的名字，如 .text, .data, .bss, 等等。
}Elf64_hdr;

typedef struct elf64_phdr {
    Elf64_Word p_type;//当前Segment的类型[4B]
    Elf64_Word p_flags;//段相关的标志[4B]
    Elf32_Off  p_offset;//当前段相对于文件起始位置的偏移量;[4B]
    Elf64_Addr      p_vaddr;//段的第一个字节将被映射到到内存中的虚拟地址[8B]
    Elf64_Addr      p_paddr;//此成员仅用于与物理地址相关的系统中。因为 System V 忽略所有应用程序的物理地址信息，此字段对与可执行文件和共享目标文件而言具体内容是指定的；[8B]
    Elf64_Xword     p_filesz;//段在文件映像中所占的字节数[8B]
    Elf64_Xword     p_memsz;//段在内存映像中占用的字节数
    Elf64_Xword     p_align;//段在文件中和内存中如何对齐
}Elf64_Phdr;

typedef struct elf64_shrd {
    Elf64_Word sh_name;//节名称在字符串表中的索引
    Elf64_Word sh_type;//节的类型和语义
    Elf64_Xword sh_flags;//1bit位的标志位

    Elf64_Addr sh_addr;//如果当前节需要被装载到内存，则当前项存储当前节映射到内存的首地址，否则应该为0；[8B]
    Elf64_Off  sh_offset;//当前节的首地址相对于文件的偏移；[8B]
    Elf64_Xword sh_size;//节的大小[8B]

    Elf64_Word sh_link;//[4B]符号表（.symtab 或 .dynsym）
    //用途：sh_link 表示符号表关联的字符串表的节头表索引。
    //    原因：符号表中的每个符号名存储在字符串表中，而 sh_link 指向该字符串表的节。
    Elf64_Word sh_info;//节的附加信息。对于特定的节有特定的含义，其他为0；[4B]
    Elf64_Xword sh_addralign;//地址约束对齐，值应该为0或者2的幂次方，0和1表示未进行对齐；[8B]
    Elf64_Xword sh_entsize;//如果该节包含一个表格（例如符号表、重定位表等），sh_entsize 表示表中每个条目的大小（以字节为单位）。
    //如果该节不是一个表格（例如.text 节），sh_entsize 通常设置为 0，因为这些节不分条目，所有数据都连续存放。

}Elf64_Shdr;

//符号表
typedef struct
{
    Elf64_Word st_name; /* Symbol name */
    unsigned char st_info; /* Type and Binding attributes *///符号的类型和属性, 高4bit标识了符号绑定(symbol binding), 低4bit标识了符号类型(symbol type), 组成符号信息(symbol information)
    unsigned char st_other; /* Reserved */
    Elf64_Half st_shndx; /* Section table index用于表示符号所在的段（Section）索引 */
    Elf64_Addr st_value; /* Symbol value */
    Elf64_Xword st_size; /* Size of object (e.g., common) */
} Elf64_Sym;

//重定位表
typedef struct
{
    Elf64_Addr r_offset; /* 引用地址（需要重定位的位置） */
    Elf64_Xword r_info;  /* 符号索引和重定位类型（编码信息） */
}Elf64_Rel;
typedef struct
{
    Elf64_Addr r_offset;     /* 引用地址（需要重定位的位置） */
    Elf64_Xword r_info;      /* 符号索引和重定位类型（编码信息） */
    Elf64_Sxword r_addend;   /* 表达式中的常量部分 */
}Elf64_Rela;


//提供动态链接器所需要的信息,
typedef struct
{
    Elf64_Sxword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
} Elf64_Dyn;
extern Elf64_Dyn _DYNAMIC[];