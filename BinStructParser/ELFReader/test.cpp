#include<fstream>  
#include<iostream>
#include<cstdio> 
#include"ELFReader.cpp"


using namespace std;
#define _CRT_SECURE_NO_WARNINGS
int main() {
    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, "C:\\Users\\11252\\Desktop桌面\\libconn.so", "rb");
    if (err != 0 || !fp) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }
    // 读取 ELF 文件头
    Elf64_hdr elf64_hdr;

    // 解析 ELF 文件头
    ElfHeaderParse(fp, &elf64_hdr);

    // 解析程序头表
    ElfProgramHeaderTableParse(fp, &elf64_hdr);

    // 解析节头表
    ElfSectionHeaderTableParse(fp, &elf64_hdr);

    //解析字符串表
   StringTableParse(fp, &elf64_hdr);

   //解析符号表
   SymbolTableParse(fp, &elf64_hdr);

   //重定位
   RelocationTableParse(fp, &elf64_hdr);

   //动态段
   printDynamicSegment64(fp, &elf64_hdr);


    // 关闭文件
    fclose(fp);

    return EXIT_SUCCESS;
	

}