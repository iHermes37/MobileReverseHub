#include"ELFReader.h"
#include<fstream>  
#include<iostream>
#include<cstdio> 

// 提取重定位项中的类型信息
#ifndef ELF64_R_TYPE
#define ELF64_R_TYPE(info) ((info) & 0xffffffff) // 低 32 位存储类型
#endif

// 提取重定位项中的符号索引
#ifndef ELF64_R_SYM
#define ELF64_R_SYM(info) ((info) >> 32) // 高 32 位存储符号索引
#endif


static void ElfHeaderParse(FILE* fp, Elf64_hdr* elf64_hdr)
{
    fseek(fp, 0, SEEK_SET);
    fread(elf64_hdr, sizeof(Elf64_hdr), 1, fp);

    printf("Magic:\t\t\t");
    for (int i = 0; i < EI_NIDENT; i++) printf("%02x ", elf64_hdr->e_indet[i]);
    printf("\n");
    printf("文件格式:\t\t");
    switch (elf64_hdr->e_type)
    {
    case 0: printf("未知的文件格式\n"); break;
    case 1: printf("重定向文件\n"); break;
    case 2: printf("可执行文件\n"); break;
    case 3: printf("共享文件\n"); break;
    case 4: printf("Core转储文件\n"); break;
    case 0xff00:printf("特定处理器的文件\n"); break;
    case 0xffff:printf("特定处理器的文件\n"); break;
    }

    printf("处理器体系结构:\t\t");
    switch (elf64_hdr->e_machine)
    {
    case 0: printf("未知体系结构\n"); break;
    case 1: printf("AT&T WE 32100\n"); break;
    case 2: printf("SPARC\n"); break;
    case 3: printf("Intel Architecture\n"); break;
    case 4: printf("Motorola 68000\n"); break;
    case 5: printf("Motorola 88000\n"); break;
    case 7: printf("Intel 80860\n"); break;
    case 8: printf("MIPS RS3000 Big-Endian\n"); break;
    case 10: printf("MIPS RS4000 Big-Endian\n"); break;
    case 62: printf("AMD x86-64 architecture\n"); break;
    case 183: printf("AArch64 architecture\n"); break;
    }

    printf("version:\t\t");
    switch (elf64_hdr->e_version) {
    case 0: printf("0\n"); break;
    case 1: printf("1\n"); break;
    }

    printf("程序入口虚拟地址:\t0x%016x\n", elf64_hdr->e_entry);
    printf("程序头表的偏移地址:\t0x%08x\n", elf64_hdr->e_phoff);
    printf("段表/节表的编译地址：\t0x%08x\n", elf64_hdr->e_shoff);

    printf("处理器标志位:\t\t%x\n", elf64_hdr->e_flags);
    printf("ELF文件头大小:\t\t%u bytes\n", elf64_hdr->e_ehsize);

    printf("程序头表的单项大小:\t%u bytes\n", elf64_hdr->e_phentsize);
    printf("程序头表的单项数量:\t%u\n", elf64_hdr->e_phnum);

    printf("节表的单项大小:\t%u bytes\n", elf64_hdr->e_shentsize);
    printf("节表的单项数量:\t%u\n", elf64_hdr->e_shnum);
    printf("字符串表在节头表中索引:\t%u\n", elf64_hdr->e_shstrndx);
}

static void ElfProgramHeaderTableParse(FILE* fp, Elf64_hdr* elf64_hdr)
{
    Elf64_Phdr phdr[99];
    fseek(fp, elf64_hdr->e_phoff, SEEK_SET);
    int count = elf64_hdr->e_phnum;
    fread(phdr, sizeof(Elf64_Phdr), count, fp);
    printf("There are %d program headers, starting at offset 0x%04x:\n\n", count, elf64_hdr->e_phoff);
    puts("程序头表:");
    puts("类型\t\t属性\t偏移量\t\t虚拟地址\t\t物理地址\t\t文件大小\t镜像大小\t对齐长度");
    for (int i = 0; i < count;i++ )
    {
        switch (phdr[i].p_type) {
        case 0:printf("PT_NULL\t"); break;//当前项未使用，项中的成员是未定义的，需要忽略当前项；
        case 1:printf("PT_LOAD\t"); break;//当前Segment是一个可装载的Segment，即可以被装载映射到内存中，其大小由p_filesz和p_memsz描述。如果p_memsz>p_filesz则剩余的字节被置零，但是p_filesz>p_memsz是非法的。动态库一般包含两个该类型的段：代码段和数据段；
        case 2:printf("PT_DYNAMIC\t"); break;//动态段，动态库特有的段，包含了动态链接必须的一些信息，比如需要链接的共享库列表、GOT等等
        case 3:printf("PT_INTERP\t"); break;//当前段用于存储一段以NULL为结尾的字符串，该字符串表明了程序解释器的位置。且当前段仅仅对于可执行文件有实际意义
        case 4:printf("PT_NOTE\t"); break;//用于保存与特定供应商或者系统相关的附加信息以便于兼容性、一致性检查，但是实际上只保存了操作系统的规范信息；
        case 5:printf("PT_SHLIB\t"); break;//保留段；
        case 6:printf("PT_PHDR\t"); break;//保存程序头表本身的位置和大小，当前段不能在文件中出现一次以上，且仅仅当程序表头为内存映像的一部分时起作用，它必须在所有加载项目之前；
            /*[PT_LPROC(0x70000000),PT_HIPROC(0x7fffffff)]：该范围内的值用作预留*/
        case 0x6474e550: printf("GNU_EH_FRAME\t"); break;
        case 0x6474e551: printf("GNU_STACK\t"); break;
        case 0x6474e552: printf("GNU_RELRO\t"); break;
        case 0x70000000: printf("PT_LOPROC\t"); break;
        case 0x7fffffff: printf("PT_HIPROC\t"); break;
        }

        putchar('\t');
        switch (phdr[i].p_flags) {//段相关的标志(权限)；
        case 0: printf("none"); break;
        case 1: printf("x"); break;
        case 2: printf("w"); break;
        case 3: printf("wx"); break;
        case 4: printf("r"); break;
        case 5: printf("rx"); break;
        case 6: printf("rw"); break;
        case 7: printf("rwx"); break;
        }

        printf("\t0x%08x", phdr[i].p_offset);
        printf("\t0x%016x", phdr[i].p_vaddr);
        printf("\t0x%016x", phdr[i].p_paddr);
        printf("\t%6u bytes", phdr[i].p_filesz);
        printf("\t%6u bytes", phdr[i].p_memsz);
        printf("\t0x%08x", phdr[i].p_align);
        putchar('\n');
    }
}

static void ElfSectionHeaderTableParse(FILE* fp, Elf64_hdr* elf64_hdr) {
    char strtable[9999];
    Elf64_Shdr shdr[99];
    fseek(fp, elf64_hdr->e_shoff, SEEK_SET);
    int count = elf64_hdr->e_shnum;
    fread(shdr, sizeof(Elf64_Shdr), count, fp);

    fseek(fp, shdr[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
    fread(strtable, 1, shdr[elf64_hdr->e_shstrndx].sh_size, fp);
    printf("There are %d section headers, starting at offset 0x%04x:\n\n", count, elf64_hdr->e_shoff);
    puts("节头表:");
    printf("[编号]\t名称\t\t\  类型\t\t属性\t虚拟地址\t\t偏移量\t\t大小\t\t索引值\t信息\t对齐长度\t表项大小\n");
    for (int i = 0; i < count; i++)
    {
        //&strtable[shdr[i].sh_name] 表示获取该字符的地址，这实际上是偏移量对应的字符串在数组中的起始地址。
        printf("[%02d]\t%s", i, &strtable[shdr[i].sh_name]);
        for (int j = 0; j < 20 - strlen(&strtable[shdr[i].sh_name]); ++j) {
            putchar(' ');
        }
        switch (shdr[i].sh_type)
        {
        case 0:printf("SHT_NULL\t"); break;//当前节是非活跃的，没有一个对应的具体的节内存；
        case 1:printf("SHT_PROGBITS\t"); break;//包含了程序的指令信息、数据等程序运行相关的信息；
        case 2:printf("SHT_SYMTAB\t"); break;//保存了符号信息，用于重定位；
        case 3:printf("SHT_STRTAB\t"); break;//一个字符串表，保存了每个节的节名称；
        case 4:printf("SHT_RELA\t"); break;//存储可重定位表项，可能会有附加内容，目标文件可能有多个可重定位表项；
        case 5:printf("SHT_HASH\t"); break;//存储符号哈希表，所有参与动态链接的目标只能包含一个哈希表，一个目标文件只能包含一个哈希表；
        case 6:printf("SHT_DYAMIC\t"); break;//存储包含动态链接的信息，一个目标文件只能包含一个；
        case 7:printf("SHT_NOTE\t"); break;//存储以某种形式标记文件的信息；
        case 8:printf("SHT_NOBITS\t"); break;//这种类型的节不占据文件空间，但是成员sh_offset依然会包含对应的偏移；
        case 9:printf("SHT_REL\t"); break;//包含可重定位表项，无附加内容，目标文件可能有多个可重定位表项；
        case 10:printf("SHT_SHLIB\t"); break;//：保留区，包含此节的程序与ABI不兼容
        case 11:printf("SHT_DYNSYM\t"); break;//保存共享库导入动态符号信息；
        case 12:printf("\t"); break;//
        case 13:printf("\t"); break;//
        case 14:printf("\t"); break;//
        case 15:printf("\t"); break;//
        case 0x70000000: printf("SHT_LOPROC"); break;
        case 0x7fffffff: printf("SHT_HIPROC"); break;
        case 0x80000000: printf("SHT_LOUSER"); break;
        case 0xffffffff: printf("SHT_HIUSER"); break;
        case 0x6ffffff6: printf("SHT_GNU_HASH"); break;
        case 0x6fffffff: printf("SHT_GNU_versym"); break;
        case 0x6ffffffe: printf("SHT_GNU_verneed"); break;
        }
        
        printf("0x%016x\t", shdr[i].sh_addr);
        printf("0x%08x\t", shdr[i].sh_offset);
        printf("%4lu bytes\t", shdr[i].sh_size);
        printf("%u\t", shdr[i].sh_link);
        printf("%u\t", shdr[i].sh_info);
        printf("%2lu bytes\t", shdr[i].sh_entsize);
        printf("%4x\n", shdr[i].sh_addralign);
    }
}

//解析每一个section中的字符串表
static void StringTableParse(FILE* fp, Elf64_hdr* elf64_hdr) {
    printf("this is stringtable \n");
    char strtable[9999];
    Elf64_Shdr shdr[99];
    uint8_t stringBuffer[9999]; // 用于保存单个字符串表
    fseek(fp, elf64_hdr->e_shoff, SEEK_SET);
    int count = elf64_hdr->e_shnum;
    fread(shdr, sizeof(Elf64_Shdr), count, fp);

    fseek(fp, shdr[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
    fread(strtable, 1, shdr[elf64_hdr->e_shstrndx].sh_size, fp);

    for (int i = 0; i < count; i++) {
        if (shdr[i].sh_type == 3) {
            printf("\t==========String Table %s==========\n", &strtable[shdr[i].sh_name]);
            fseek(fp, shdr[i].sh_offset, SEEK_SET);
            fread(stringBuffer, 1, shdr[i].sh_size, fp);
            uint32_t pos = 0;
            while (pos < shdr[i].sh_size) {
                printf("\t%s\n", stringBuffer + pos);
                pos += strlen((char*)(stringBuffer + pos)) + 1; // 跳过当前字符串（加上末尾 '\0'）
            }
        }
    }
    printf("ELF String Table End\n");
}

// Print Symbol Table
static const char* getSymbolBindingString(uint8_t symbolBinding) {
    switch (symbolBinding) {
    case 0:        return "LOCAL";
    case 1:       return "GLOBAL";
    case 2:         return "WEAK";
    /*case STB_NUM:          return "STB_NUM";
    case STB_GNU_UNIQUE:   return "GNU_UNIQUE";*/
    case 12:         return "STB_HIOS";
    case 13:       return "STB_LOPROC";
    case 15:       return "STB_HIPROC";
    default:               return "UNKNOWN";
    }
}
static const char* getSymbolTypeString(uint8_t symbolType) {
    switch (symbolType) {
    case 0:    return "NOTYPE";
    case 1:    return "OBJECT";
    case 2:      return "FUNC";
    case 3:   return "SECTION";
    case 4:      return "FILE";
    //case STT_COMMON:    return "COMMON";
    //case STT_TLS:       return "TLS";
    //case STT_NUM:       return "STT_NUM";
    //case STT_GNU_IFUNC: return "GNU_IFUNC";
    case 10:      return "LOOS";
    case 12:      return "HIOS";
    case 13:    return "LOPROC";
    case 15:    return "HIPROC";
    default:            return "UNKNOWN";
    }
}
static const char* getSymbolVisibility(uint8_t st_other) {
    unsigned char visibility = st_other & 0x03;
    switch (visibility) {
    case 0:            return "DEFAULT";
    case 1:            return "INTERNAL";
    case 2:            return "HIDDEN";
    case 3:            return "PROTECTED";
    default:           return "UNKNOWN";
    }
}


//解析符号表 
//.dynsym 动态链接符号表
//.symtab 符号表
static void SymbolTableParse(FILE* fp, Elf64_hdr* elf64_hdr) {
    char strtable[9999];
    char str_table[9999];
    Elf64_Shdr shdr[99];
    fseek(fp, elf64_hdr->e_shoff, SEEK_SET);
    int count = elf64_hdr->e_shnum;
    if (fread(shdr, sizeof(Elf64_Shdr), count, fp) != count) {
        perror("Failed to read section headers");
        return;
    }

    fseek(fp, shdr[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
    if (fread(str_table, 1, shdr[elf64_hdr->e_shstrndx].sh_size, fp) != shdr[elf64_hdr->e_shstrndx].sh_size) {
        perror("Failed to read string table");
        return;
    }

    fseek(fp, shdr[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
    fread(strtable, 1, shdr[elf64_hdr->e_shstrndx].sh_size, fp);

    for (int i = 0; i < count; i++) {
        if (shdr[i].sh_type == 2 || shdr[i].sh_type == 11) {
            printf("\t==========symbol Table %s==========\n", &strtable[shdr[i].sh_name]);
            if (shdr[i].sh_entsize == 0) continue;
            Elf64_Xword symbolnum = shdr[i].sh_size / shdr[i].sh_entsize;

            // 加载符号表段
            Elf64_Sym* pSymbolTable = (Elf64_Sym*)malloc(shdr[i].sh_size);
            fseek(fp, shdr[i].sh_offset, SEEK_SET);
            fread(pSymbolTable, 1, shdr[i].sh_size, fp);

            // 加载关联的字符串表
            char* str_table = (char*)malloc(shdr[shdr[i].sh_link].sh_size);
            fseek(fp, shdr[shdr[i].sh_link].sh_offset, SEEK_SET);
            fread(str_table, 1, shdr[shdr[i].sh_link].sh_size, fp);

            printf("\tNum \tValue\t\tSize\t\tIndex\t\tName\n");
            for (int j = 0; j < symbolnum; j++) {
                printf("\t%04d", j);
                printf("\t%08lx", pSymbolTable[j].st_value);
                printf("\t%08lx", pSymbolTable[j].st_size);
                printf("\t%04x", pSymbolTable[j].st_shndx);
                printf("\t%s\n", str_table + pSymbolTable[j].st_name);
            }

            free(pSymbolTable);
            free(str_table);
        }
    }
}
//错误代码（偏移量基于 shdr 的内存地址）
//char* str_table = (char*)(shdr)+shdr[shdr[i].sh_link].sh_offset; // 错误
//问题描述：
//
//shdr 是内存中段表的起始地址，而 sh_offset 是文件偏移量。
//将文件偏移量 sh_offset 直接加到内存地址 shdr 上，结果会导致访问错误的内存地址。
//如果文件偏移量的值很大（例如远超 shdr 的内存范围），程序可能会发生 访问冲突（segmentation fault）。


static const char* getRelocationTypeString64(Elf64_Word value) {
    switch (value) {
    case 0: return "R_386_NONE";
    case 1: return "R_386_32";
    case 2: return "R_386_PC32";
    case 3: return "R_386_GOT32";
    case 4: return "R_386_PLT32";
    case 5: return "R_386_COPY";
    case 6: return "R_386_GLOB_DAT";
    case 7: return "R_386_JMP_SLOT";
    case 8: return "R_386_RELATIVE";
    case 9: return "R_386_GOTOFF";
    case 10: return "R_386_GOTPC";
    case 11: return "R_386_32PLT";
    case 14: return "R_386_TLS_TPOFF";
    case 15: return "R_386_TLS_IE";
    case 16: return "R_386_TLS_GOTIE";
    case 17: return "R_386_TLS_LE";
    case 18: return "R_386_TLS_GD";
    case 19: return "R_386_TLS_LDM";
    case 20: return "R_386_16";
    case 21: return "R_386_PC16";
    case 22: return "R_386_8";
    case 23: return "R_386_PC8";
    case 24: return "R_386_TLS_GD_32";
    case 25: return "R_386_TLS_GD_PUSH";
    case 26: return "R_386_TLS_GD_CALL";
    case 27: return "R_386_TLS_GD_POP";
    case 28: return "R_386_TLS_LDM_32";
    case 29: return "R_386_TLS_LDM_PUSH";
    case 30: return "R_386_TLS_LDM_CALL";
    case 31: return "R_386_TLS_LDM_POP";
    case 32: return "R_386_TLS_LDO_32";
    case 33: return "R_386_TLS_IE_32";
    case 34: return "R_386_TLS_LE_32";
    case 35: return "R_386_TLS_DTPMOD32";
    case 36: return "R_386_TLS_DTPOFF32";
    case 37: return "R_386_TLS_TPOFF32";
    case 38: return "R_386_SIZE32";
    case 39: return "R_386_TLS_GOTDESC";
    case 40: return "R_386_TLS_DESC_CALL";
    case 41: return "R_386_TLS_DESC";
    case 42: return "R_386_IRELATIVE";
    case 43: return "R_386_GOT32X";
    default: return "Unknown relocation type";
    }
}
//解析重定位表
//static void RelocationTableParse(FILE* fp, Elf64_hdr* elf64_hdr) {
//    int sectionNum = elf64_hdr->e_shnum;
//    Elf64_Shdr shdr[99];
//    fseek(fp, elf64_hdr->e_shoff, SEEK_SET);
//    int count = elf64_hdr->e_shnum;
//    fread(shdr, sizeof(Elf64_Shdr), count, fp);
//
//    printf("Relocation Tables:\n");
//    for (int i = 0; i < sectionNum; i++) {
//        if (shdr[i].sh_type == 4) {
//
//            Elf64_Shdr* pRelocationTableHeader = &shdr[i];
//            Elf64_Rel* pRelocationTable = (Elf64_Rel*)malloc(pRelocationTableHeader->sh_size);
//            Elf64_Word relocItemNum = pRelocationTableHeader->sh_size / pRelocationTableHeader->sh_entsize;
//            // relocation table sh_link is index of symbol table header
//            Elf64_Shdr* pSymbolTableHeader = (Elf64_Shdr*)&shdr[shdr[i].sh_link];
//            //real symbol table
//            Elf64_Sym* pSymbolTable = (Elf64_Sym*)malloc(pSymbolTableHeader->sh_size);
//            //string table for symbol name
//            char* pSymbolTableStringTable = (char*)malloc(shdr[pSymbolTableHeader->sh_link].sh_size);
//
//            char strtable[9999];
//            fseek(fp, shdr[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
//            fread(strtable, 1, shdr[elf64_hdr->e_shstrndx].sh_size, fp);
//
//            printf("Relocation Section '%s' at offset contains %d entries\n", (char*)strtable + shdr[i].sh_name, relocItemNum);
//            printf("\tOffset\t\tInfo\t\tType\t\t\t\tSym.value\t\tSym.name\n");
//            for (int j = 0; j < relocItemNum; j++) {
//                printf("\t%08x", pRelocationTable[j].r_offset);
//                printf("\t%08x", pRelocationTable[j].r_info);
//                printf("\t%s\t", getRelocationTypeString32(ELF64_R_TYPE(pRelocationTable[j].r_info)));
//                printf("\t%08x\t", pSymbolTable[ELF64_R_SYM(pRelocationTable[j].r_info)].st_value);
//                //R_SYM get the index of symbol in symbol table, st_name is index of symbol name in string table
//                printf("\t%s", &pSymbolTableStringTable[pSymbolTable[ELF64_R_SYM(pRelocationTable[j].r_info)].st_name]);
//                printf("\n");
//            }
//        }
//    }
//}

static void RelocationTableParse(FILE* fp, Elf64_hdr* elf64_hdr) {
    int sectionNum = elf64_hdr->e_shnum;
    Elf64_Shdr shdr[99];
    fseek(fp, elf64_hdr->e_shoff, SEEK_SET);
    fread(shdr, sizeof(Elf64_Shdr), sectionNum, fp);

    printf("Relocation Tables:\n");
    for (int i = 0; i < sectionNum; i++) {
        if (shdr[i].sh_type == 4) { // 检查是否为重定位表
            Elf64_Shdr* pRelocationTableHeader = &shdr[i];
            Elf64_Rel* pRelocationTable = (Elf64_Rel*)malloc(pRelocationTableHeader->sh_size);
            fseek(fp, pRelocationTableHeader->sh_offset, SEEK_SET);
            fread(pRelocationTable, pRelocationTableHeader->sh_size, 1, fp);

            Elf64_Word relocItemNum = pRelocationTableHeader->sh_size / pRelocationTableHeader->sh_entsize;

            // 符号表头部
            Elf64_Shdr* pSymbolTableHeader = &shdr[pRelocationTableHeader->sh_link];
            Elf64_Sym* pSymbolTable = (Elf64_Sym*)malloc(pSymbolTableHeader->sh_size);
            fseek(fp, pSymbolTableHeader->sh_offset, SEEK_SET);
            fread(pSymbolTable, pSymbolTableHeader->sh_size, 1, fp);

            // 符号表的字符串表
            char* pSymbolTableStringTable = (char*)malloc(shdr[pSymbolTableHeader->sh_link].sh_size);
            fseek(fp, shdr[pSymbolTableHeader->sh_link].sh_offset, SEEK_SET);
            fread(pSymbolTableStringTable, shdr[pSymbolTableHeader->sh_link].sh_size, 1, fp);

            // 段名字符串表
            char strtable[9999];
            fseek(fp, shdr[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
            fread(strtable, 1, shdr[elf64_hdr->e_shstrndx].sh_size, fp);

            printf("Relocation Section '%s' at offset contains %d entries\n", (char*)strtable + shdr[i].sh_name, relocItemNum);
            printf("\tOffset\t\tInfo\t\tType\t\t\t\tSym.value\t\tSym.name\n");

            for (int j = 0; j < relocItemNum; j++) {
                printf("\t%08lx", pRelocationTable[j].r_offset);
                printf("\t%08lx", pRelocationTable[j].r_info);
                printf("\t%s\t", getRelocationTypeString64(ELF64_R_TYPE(pRelocationTable[j].r_info)));
                printf("\t%08lx\t", pSymbolTable[ELF64_R_SYM(pRelocationTable[j].r_info)].st_value);
                printf("\t%s\n", &pSymbolTableStringTable[pSymbolTable[ELF64_R_SYM(pRelocationTable[j].r_info)].st_name]);
            }

            // 释放动态内存
            free(pRelocationTable);
            free(pSymbolTable);
            free(pSymbolTableStringTable);
        }
    }
}


// Print Dynamic Segment
#define DT_VAL 0
#define DT_PTR 1
//const char* getDynamicType(Elf_Xword value) {
//    if (value >= DT_LOOS && value <= DT_HIOS)
//        return "OS-Specific";
//    if (value >= DT_LOPROC && value <= DT_HIPROC)
//        return "Processor-Specific";
//    switch (value) {
//    case DT_NULL: return "NULL";
//    case DT_NEEDED: return "NEEDED";
//    case DT_PLTRELSZ: return "PLTRELSZ";
//    case DT_PLTGOT: return "PLTGOT";
//    case DT_HASH: return "HASH";
//    case DT_STRTAB: return "STRTAB";
//    case DT_SYMTAB: return "SYMTAB";
//    case DT_RELA: return "RELA";
//    case DT_RELASZ: return "RELASZ";
//    case DT_RELAENT: return "RELAENT";
//    case DT_STRSZ: return "STRSZ";
//    case DT_SYMENT: return "SYMENT";
//    case DT_INIT: return "INIT";
//    case DT_FINI: return "FINI";
//    case DT_SONAME: return "SONAME";
//    case DT_RPATH: return "RPATH";
//    case DT_SYMBOLIC: return "SYMBOLIC";
//    case DT_REL: return "REL";
//    case DT_RELSZ: return "RELSZ";
//    case DT_RELENT: return "RELENT";
//    case DT_PLTREL: return "PLTREL";
//    case DT_DEBUG: return "DEBUG";
//    case DT_TEXTREL: return "TEXTREL";
//    case DT_JMPREL: return "JMPREL";
//    case DT_BIND_NOW: return "BIND_NOW";
//    case DT_INIT_ARRAY: return "INIT_ARRAY";
//    case DT_FINI_ARRAY: return "FINI_ARRAY";
//    case DT_INIT_ARRAYSZ: return "INIT_ARRAYSZ";
//    case DT_FINI_ARRAYSZ: return "FINI_ARRAYSZ";
//    case DT_RUNPATH: return "RUNPATH";
//    case DT_FLAGS: return "FLAGS";
//    case DT_ENCODING: return "ENCODING";
//    case DT_SYMTAB_SHNDX: return "SYMTAB_SHNDX";
//    case DT_RELRSZ: return "RELRSZ";
//    case DT_RELR: return "RELR";
//    case DT_RELRENT: return "RELRENT";
//    case DT_NUM: return "NUM";
//    case DT_VALRNGLO: return "VALRNGLO";
//    case DT_GNU_PRELINKED: return "GNU_PRELINKED";
//    case DT_GNU_CONFLICTSZ: return "GNU_CONFLICTSZ";
//    case DT_GNU_LIBLISTSZ: return "GNU_LIBLISTSZ";
//    case DT_CHECKSUM: return "CHECKSUM";
//    case DT_PLTPADSZ: return "PLTPADSZ";
//    case DT_MOVEENT: return "MOVEENT";
//    case DT_MOVESZ: return "MOVESZ";
//    case DT_FEATURE_1: return "FEATURE_1";
//    case DT_POSFLAG_1: return "POSFLAG_1";
//    case DT_SYMINSZ: return "SYMINSZ";
//    case DT_SYMINENT: return "SYMINENT";
//    case DT_ADDRRNGLO: return "ADDRRNGLO";
//    case DT_GNU_HASH: return "GNU_HASH";
//    case DT_TLSDESC_PLT: return "TLSDESC_PLT";
//    case DT_TLSDESC_GOT: return "TLSDESC_GOT";
//    case DT_GNU_CONFLICT: return "GNU_CONFLICT";
//    case DT_GNU_LIBLIST: return "GNU_LIBLIST";
//    case DT_CONFIG: return "CONFIG";
//    case DT_DEPAUDIT: return "DEPAUDIT";
//    case DT_AUDIT: return "AUDIT";
//    case DT_PLTPAD: return "PLTPAD";
//    case DT_MOVETAB: return "MOVETAB";
//    case DT_SYMINFO: return "SYMINFO";
//    case DT_VERSYM: return "VERSYM";
//    case DT_RELACOUNT: return "RELACOUNT";
//    case DT_RELCOUNT: return "RELCOUNT";
//    case DT_FLAGS_1: return "FLAGS_1";
//    case DT_VERDEF: return "VERDEF";
//    case DT_VERDEFNUM: return "VERDEFNUM";
//    case DT_VERNEED: return "VERNEED";
//    case DT_VERNEEDNUM: return "VERNEEDNUM";
//    case DT_AUXILIARY: return "AUXILIARY";
//    case DT_FILTER: return "FILTER";
//    default: return "Unknown Type";
//    }
//}
//uint32_t getDynamicDunType(Elf_Xword value) {
//    switch (value) {
//    case DT_NULL:
//    case DT_NEEDED:
//    case DT_PLTRELSZ:
//    case DT_RELASZ:
//    case DT_RELAENT:
//    case DT_STRSZ:
//    case DT_SYMENT:
//    case DT_SONAME:
//    case DT_RPATH:
//    case DT_SYMBOLIC:
//    case DT_RELSZ:
//    case DT_RELENT:
//    case DT_PLTREL:
//    case DT_TEXTREL:
//    case DT_BIND_NOW:
//    case DT_LOPROC:
//    case DT_HIPROC:
//        return DT_VAL;
//    case DT_PLTGOT:
//    case DT_HASH:
//    case DT_STRTAB:
//    case DT_SYMTAB:
//    case DT_RELA:
//    case DT_INIT:
//    case DT_FINI:
//    case DT_JMPREL:
//    case DT_DEBUG:
//    case DT_REL:
//        return DT_PTR;
//    default:
//        return DT_VAL;
//    }
//}

static const char* getDynamicType(Elf64_Xword value) {
    if (value >= 0x60000000 && value <= 0x6fffffff) // DT_LOOS 和 DT_HIOS 的范围
        return "OS-Specific";
    if (value >= 0x70000000 && value <= 0x7fffffff) // DT_LOPROC 和 DT_HIPROC 的范围
        return "Processor-Specific";
    switch (value) {
    case 0: return "NULL";  // DT_NULL
    case 1: return "NEEDED";  // DT_NEEDED
    case 2: return "PLTRELSZ";  // DT_PLTRELSZ
    case 3: return "PLTGOT";  // DT_PLTGOT
    case 4: return "HASH";  // DT_HASH
    case 5: return "STRTAB";  // DT_STRTAB
    case 6: return "SYMTAB";  // DT_SYMTAB
    case 7: return "RELA";  // DT_RELA
    case 8: return "RELASZ";  // DT_RELASZ
    case 9: return "RELAENT";  // DT_RELAENT
    case 10: return "STRSZ";  // DT_STRSZ
    case 11: return "SYMENT";  // DT_SYMENT
    case 12: return "INIT";  // DT_INIT
    case 13: return "FINI";  // DT_FINI
    case 14: return "SONAME";  // DT_SONAME
    case 15: return "RPATH";  // DT_RPATH
    case 16: return "SYMBOLIC";  // DT_SYMBOLIC
    case 17: return "REL";  // DT_REL
    case 18: return "RELSZ";  // DT_RELSZ
    case 19: return "RELENT";  // DT_RELENT
    case 20: return "PLTREL";  // DT_PLTREL
    case 21: return "DEBUG";  // DT_DEBUG
    case 22: return "TEXTREL";  // DT_TEXTREL
    case 23: return "JMPREL";  // DT_JMPREL
    case 24: return "BIND_NOW";  // DT_BIND_NOW
    case 25: return "INIT_ARRAY";  // DT_INIT_ARRAY
    case 26: return "FINI_ARRAY";  // DT_FINI_ARRAY
    case 27: return "INIT_ARRAYSZ";  // DT_INIT_ARRAYSZ
    case 28: return "FINI_ARRAYSZ";  // DT_FINI_ARRAYSZ
    case 29: return "RUNPATH";  // DT_RUNPATH
    case 30: return "FLAGS";  // DT_FLAGS
    case 31: return "ENCODING";  // DT_ENCODING
    case 32: return "SYMTAB_SHNDX";  // DT_SYMTAB_SHNDX
    case 33: return "RELRSZ";  // DT_RELRSZ
    case 34: return "RELR";  // DT_RELR
    case 35: return "RELRENT";  // DT_RELRENT
    case 36: return "NUM";  // DT_NUM
    case 37: return "VALRNGLO";  // DT_VALRNGLO
    case 38: return "GNU_PRELINKED";  // DT_GNU_PRELINKED
    case 39: return "GNU_CONFLICTSZ";  // DT_GNU_CONFLICTSZ
    case 40: return "GNU_LIBLISTSZ";  // DT_GNU_LIBLISTSZ
    case 41: return "CHECKSUM";  // DT_CHECKSUM
    case 42: return "PLTPADSZ";  // DT_PLTPADSZ
    case 43: return "MOVEENT";  // DT_MOVEENT
    case 44: return "MOVESZ";  // DT_MOVESZ
    case 45: return "FEATURE_1";  // DT_FEATURE_1
    case 46: return "POSFLAG_1";  // DT_POSFLAG_1
    case 47: return "SYMINSZ";  // DT_SYMINSZ
    case 48: return "SYMINENT";  // DT_SYMINENT
    case 49: return "ADDRRNGLO";  // DT_ADDRRNGLO
    case 50: return "GNU_HASH";  // DT_GNU_HASH
    case 51: return "TLSDESC_PLT";  // DT_TLSDESC_PLT
    case 52: return "TLSDESC_GOT";  // DT_TLSDESC_GOT
    case 53: return "GNU_CONFLICT";  // DT_GNU_CONFLICT
    case 54: return "GNU_LIBLIST";  // DT_GNU_LIBLIST
    case 55: return "CONFIG";  // DT_CONFIG
    case 56: return "DEPAUDIT";  // DT_DEPAUDIT
    case 57: return "AUDIT";  // DT_AUDIT
    case 58: return "PLTPAD";  // DT_PLTPAD
    case 59: return "MOVETAB";  // DT_MOVETAB
    case 60: return "SYMINFO";  // DT_SYMINFO
    case 61: return "VERSYM";  // DT_VERSYM
    case 62: return "RELACOUNT";  // DT_RELACOUNT
    case 63: return "RELCOUNT";  // DT_RELCOUNT
    case 64: return "FLAGS_1";  // DT_FLAGS_1
    case 65: return "VERDEF";  // DT_VERDEF
    case 66: return "VERDEFNUM";  // DT_VERDEFNUM
    case 67: return "VERNEED";  // DT_VERNEED
    case 68: return "VERNEEDNUM";  // DT_VERNEEDNUM
    case 69: return "AUXILIARY";  // DT_AUXILIARY
    case 70: return "FILTER";  // DT_FILTER
    default: return "Unknown Type";  // Unknown DT type
    }
}

static uint32_t getDynamicDunType(Elf64_Xword value) {
    switch (value) {
    case 0: // DT_NULL
    case 1: // DT_NEEDED
    case 2: // DT_PLTRELSZ
    case 8: // DT_RELASZ
    case 9: // DT_RELAENT
    case 10: // DT_STRSZ
    case 11: // DT_SYMENT
    case 14: // DT_SONAME
    case 15: // DT_RPATH
    case 16: // DT_SYMBOLIC
    case 18: // DT_RELSZ
    case 19: // DT_RELENT
    case 20: // DT_PLTREL
    case 21: // DT_TEXTREL
    case 24: // DT_BIND_NOW
    case 0x70000000: // DT_LOPROC
    case 0x7fffffff: // DT_HIPROC
        return 0; // DT_VAL
    case 3: // DT_PLTGOT
    case 4: // DT_HASH
    case 5: // DT_STRTAB
    case 6: // DT_SYMTAB
    case 7: // DT_RELA
    case 12: // DT_INIT
    case 13: // DT_FINI
    case 23: // DT_JMPREL
    //case 21: // DT_DEBUG
    case 17: // DT_REL
        return 1; // DT_PTR
    default:
        return 0; // DT_VAL
    }
}

//static void printDynamicSegment64(FILE* fp, Elf64_hdr* elf64_hdr) {
//    // 获取 ELF 文件中的节头表信息
//    Elf64_Shdr* pSectionHeader = (Elf64_Shdr*)malloc(elf64_hdr->e_shnum * sizeof(Elf64_Shdr));
//    fseek(fp, elf64_hdr->e_shoff, SEEK_SET);
//    fread(pSectionHeader, sizeof(Elf64_Shdr), elf64_hdr->e_shnum, fp);
//
//    // 获取节头字符串表
//    char* pSectionHeaderStringTable = (char*)malloc(pSectionHeader[elf64_hdr->e_shstrndx].sh_size);
//    fseek(fp, pSectionHeader[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
//    fread(pSectionHeaderStringTable, 1, pSectionHeader[elf64_hdr->e_shstrndx].sh_size, fp);
//
//    // 处理动态节
//    for (int i = 0; i < elf64_hdr->e_shnum; i++) {
//        if (pSectionHeader[i].sh_type == 6) {
//            Elf64_Shdr* pDynamicSection = &pSectionHeader[i];
//            Elf64_Word dynamicItemNum = pDynamicSection->sh_size / pDynamicSection->sh_entsize;
//            printf("Dynamic Section At File Offset %#lx Contains %d Entries:\n", pDynamicSection->sh_offset, dynamicItemNum);
//            printf("\tTag \t\tType\t\t\t\tName/Value\n");
//
//            // 动态段表项
//            Elf64_Dyn* pDynamicTable = (Elf64_Dyn*)malloc(pDynamicSection->sh_size);
//
//            // 获取动态字符串表
//            Elf64_Shdr* pDynamicStringTableHeader = &pSectionHeader[pDynamicSection->sh_link];
//            char* pDynamicStringTable = (char*)malloc(pDynamicStringTableHeader->sh_size);
//
//            for (int j = 0; j < dynamicItemNum; j++) {
//                printf("\t%08lx", pDynamicTable[j].d_tag);
//                printf("\t%-16s", getDynamicType(pDynamicTable[j].d_tag));
//                printf("\t%08lx\t", pDynamicTable[j].d_un.d_val);
//                if (getDynamicDunType(pDynamicTable[j].d_tag) == DT_PTR) {
//                    printf("(PTR)");
//                }
//                // 如果是共享库路径或者 soname，则输出字符串
//                switch (pDynamicTable[j].d_tag) {
//                case 1://DT_NEEDED
//                case 14://DT_SONAME
//                    printf("[%s]", pDynamicStringTable + pDynamicTable[j].d_un.d_val);
//                    break;
//                default:
//                    break;
//                }
//                printf("\n");
//            }
//        }
//    }
//
//    free(pSectionHeader);
//    free(pSectionHeaderStringTable);
//}


static void printDynamicSegment64(FILE* fp, Elf64_hdr* elf64_hdr) {
    // 获取 ELF 文件中的节头表信息
    Elf64_Shdr* pSectionHeader = (Elf64_Shdr*)malloc(elf64_hdr->e_shnum * sizeof(Elf64_Shdr));
    if (!pSectionHeader) {
        perror("Memory allocation for section header failed");
        return;
    }

    fseek(fp, elf64_hdr->e_shoff, SEEK_SET);
    fread(pSectionHeader, sizeof(Elf64_Shdr), elf64_hdr->e_shnum, fp);

    // 获取节头字符串表
    char* pSectionHeaderStringTable = (char*)malloc(pSectionHeader[elf64_hdr->e_shstrndx].sh_size);
    if (!pSectionHeaderStringTable) {
        perror("Memory allocation for section header string table failed");
        free(pSectionHeader);
        return;
    }

    fseek(fp, pSectionHeader[elf64_hdr->e_shstrndx].sh_offset, SEEK_SET);
    fread(pSectionHeaderStringTable, 1, pSectionHeader[elf64_hdr->e_shstrndx].sh_size, fp);

    // 处理动态节
    for (int i = 0; i < elf64_hdr->e_shnum; i++) {
        if (pSectionHeader[i].sh_type == 6) {  // SHT_DYNAMIC == 6
            Elf64_Shdr* pDynamicSection = &pSectionHeader[i];
            Elf64_Word dynamicItemNum = pDynamicSection->sh_size / pDynamicSection->sh_entsize;
            printf("Dynamic Section At File Offset %#lx Contains %d Entries:\n", pDynamicSection->sh_offset, dynamicItemNum);
            printf("\tTag \t\tType\t\t\t\tName/Value\n");

            // 动态段表项
            Elf64_Dyn* pDynamicTable = (Elf64_Dyn*)malloc(pDynamicSection->sh_size);
            if (!pDynamicTable) {
                perror("Memory allocation for dynamic table failed");
                free(pSectionHeader);
                free(pSectionHeaderStringTable);
                return;
            }

            // 获取动态字符串表
            Elf64_Shdr* pDynamicStringTableHeader = &pSectionHeader[pDynamicSection->sh_link];
            char* pDynamicStringTable = (char*)malloc(pDynamicStringTableHeader->sh_size);
            if (!pDynamicStringTable) {
                perror("Memory allocation for dynamic string table failed");
                free(pSectionHeader);
                free(pSectionHeaderStringTable);
                free(pDynamicTable);
                return;
            }

            // 读取动态表项和字符串表数据
            fseek(fp, pDynamicSection->sh_offset, SEEK_SET);
            fread(pDynamicTable, sizeof(Elf64_Dyn), dynamicItemNum, fp);

            fseek(fp, pDynamicStringTableHeader->sh_offset, SEEK_SET);
            fread(pDynamicStringTable, 1, pDynamicStringTableHeader->sh_size, fp);

            // 打印动态表项
            for (int j = 0; j < dynamicItemNum; j++) {
                printf("\t%08lx", pDynamicTable[j].d_tag);
                printf("\t%-16s", getDynamicType(pDynamicTable[j].d_tag));
                printf("\t%08lx\t", pDynamicTable[j].d_un.d_val);

                // 判断指针类型并输出特殊处理
                if (getDynamicDunType(pDynamicTable[j].d_tag) == DT_PTR) {
                    printf("(PTR)");
                }

                // 如果是共享库路径或者 soname，则输出字符串
                switch (pDynamicTable[j].d_tag) {
                case 1: // DT_NEEDED
                case 14: // DT_SONAME
                    printf("[%s]", pDynamicStringTable + pDynamicTable[j].d_un.d_val);
                    break;
                default:
                    break;
                }
                printf("\n");
            }

            // 释放动态表和字符串表内存
            free(pDynamicTable);
            free(pDynamicStringTable);
        }
    }

    // 释放节头表和节头字符串表内存
    free(pSectionHeader);
    free(pSectionHeaderStringTable);
}





//static void SectionTypeParse(FILE* fp, Elf64_hdr* elf64_hdr) {
//       
//}
