#pragma once
#include <stdint.h>
#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;
//byte	8 位有符号整数
//ubyte	8 位无符号整数
//short	16 位有符号整数，采用小端字节序
//ushort	16 位无符号整数，采用小端字节序
//int	32 位有符号整数，采用小端字节序
//uint	32 位无符号整数，采用小端字节序
//long	64 位有符号整数，采用小端字节序
//ulong	64 位无符号整数，采用小端字节序
//sleb128	有符号 LEB128，可变长度
//uleb128	无符号 LEB128，可变长度
//uleb128p1	无符号 LEB128 加 1，可变长度

#define kSHA1DigestLen 20  // SHA-1 哈希值长度是 20 字节

//byte
typedef int8_t s1;
typedef uint8_t u1;
//short
typedef int16_t s2;
typedef uint16_t u2;
//int
typedef int32_t s4;
typedef uint32_t u4;
//long
typedef int64_t s8;
typedef uint64_t u8;
//leb,由于可变长度，后续使用动态数组存储 uleb128 编码的字节

 //---------文件头-------------------
struct DexHeader {
    u1  magic[8];           //标识 DEX 文件，其中 DEX_FILE_MAGIC ="dex\n035\0"
    u4  checksum;          //除 magic 和此字段之外的文件剩下内容的 adler32 校验和，用于检测文件损坏情况
    u1  signature[kSHA1DigestLen]; //除 magic、checksum 和此字段之外的文件的内容的 SHA-1 签名（哈希），用于对文件进行唯一标识
    u4  fileSize;           //整个文件（包括文件头）的大小，以字节为单位
    u4  headerSize;         //文件头的大小，以字节为单位。
    u4  endianTag;          //字节序标记，大端序或者小端序。
    u4  linkSize;           //如果此文件未进行静态链接，则该值为 0，反之为链接区段的大小，
    u4  linkOff;            //如果 link_size == 0，则该值为 0； 反之，该偏移量是文件开头到到 link_data 区段的偏移量。
    u4  mapOff;             //该偏移量必须非零，标识从文件开头到 data 区段的偏移量。
    u4  stringIdsSize;      //字符串标识符列表中的字符串数量
    u4  stringIdsOff;       //如果 string_ids_size == 0（不可否认是一种奇怪的极端情况），则该值为 0； 反之表示从文件开头到string_ids的偏移量。
    u4  typeIdsSize;        //类型标识符列表中的元素数量，最大为 65535
    u4  typeIdsOff;         //如果 type_ids_size == 0（不可否认是一种奇怪的极端情况），则该值为 0； 反之表示从文件开头到 type_ids 区段开头的偏移量。
    u4  protoIdsSize;       //原型（方法）标识符列表中的元素数量，最多为 65535
    u4  protoIdsOff;        //如果 proto_ids_size == 0（不可否认是一种奇怪的极端情况），则该值为 0； 反之该偏移量表示文件开头到 proto_ids 区段开头的偏移量。
    u4  fieldIdsSize;       //字段标识符列表中的元素数量
    u4  fieldIdsOff;        //如果 field_ids_size == 0，则该值为 0； 反之该偏移量表示文件开头到 field_ids 区段开头的偏移量。
    u4  methodIdsSize;      //方法标识符列表中的元素数量
    u4  methodIdsOff;       //如果 method_ids_size == 0，则该值为 0。反之该偏移量表示从文件开头到 method_ids 区段开头的偏移量。
    u4  classDefsSize;      //类定义列表中的元素数量
    u4  classDefsOff;       //如果 class_defs_size == 0（不可否认是一种奇怪的极端情况），则该值为 0 ；反之该偏移量表示文件开头到 class_defs 区段开头的偏移量。
    u4  dataSize;           //data 区段的以字节为单位的大小，必须是 sizeof(uint) 的偶数倍，说明 8 字节对齐。
    u4  dataOff;            //从文件开头到 data 区段开头的偏移量。
};

//----------索引区------------------

//StringIds 区
struct DexStringId {
    u4 stringDataOff;   /* 字符串数据偏移，也就是数据区中各个 StringData 的文件偏移*/
};

//type_ids 区
struct DexTypeId {
    u4 descriptorIdx;    /* 指向 DexStringId列表的索引 */
};

//Proto id 字段
struct DexProtoId {
    u4 shortyIdx;       /* 返回类型+参数类型，简写，指向DexStringId列表的索引 */
    u4 returnTypeIdx;   /* 返回类型，指向DexTypeId列表的索引 */
    u4 parametersOff;   /* 参数类型，指向DexTypeList的偏移 */
};

struct DexTypeItem {
    u2 typeIdx;           /* 参数类型，指向DexTypeId列表的索引，最终指向字符串索引 */
};

struct DexTypeList {
    u4 size;             /* DexTypeItem的个数，即参数个数 */
    DexTypeItem list[1]; /* 指向DexTypeItem开始处 */
};

//field id 区
struct DexFieldId {
    u2 classIdx;   /* 类的类型，指向DexTypeId列表的索引 */
    u2 typeIdx;    /* 字段类型，指向DexTypeId列表的索引 */
    u4 nameIdx;    /* 字段名，指向DexStringId列表的索引 */
};

//method id 区
struct DexMethodId {
    u2 classIdx;  /* 类的类型，指向DexTypeId列表的索引 */
    u2 protoIdx;  /* 声明类型，指向DexProtoId列表的索引 */
    u4 nameIdx;   /* 方法名，  指向DexStringId列表的索引 */
};

// 类的字段与方法概况
// 类的基本信息-------------------------------------------
struct DexClassDef {
    u4 classIdx;    /* 类的类型，指向DexTypeId列表的索引 */
    u4 accessFlags; /* 访问标志 */
    u4 superclassIdx;  /* 父类类型，指向DexTypeId列表的索引 */
    u4 interfacesOff; /* 接口，指向DexTypeList的偏移 */
    u4 sourceFileIdx; /* 源文件名，指向DexStringId列表的索引 */
    u4 annotationsOff; /* 注解，指向DexAnnotationsDirectoryItem结构 */
    u4 classDataOff;   /* 指向DexClassData结构的偏移 */
    u4 staticValuesOff;  /* 指向DexEncodedArray结构的偏移 */
};

// 详细描述类的字段个数与方法个数
struct DexClassDataHeader {
    u4 staticFieldsSize;  /* 静态字段个数 */
    u4 instanceFieldsSize; /* 实例字段个数 */
    u4 directMethodsSize;  /* 直接方法个数 */
    u4 virtualMethodsSize; /* 虚方法个数 */
};

// 字段定义
struct DexField {
    u4 fieldIdx;    /* 指向DexFieldId的索引 uleb128*/
    u4 accessFlags; /* 访问标志 uleb128*/
};

// 方法定义
struct DexMethod {
    u4 methodIdx;   /* 指向DexMethodId的索引 uleb128*/
    u4 accessFlags; /* 访问标志 uleb128*/
    u4 codeOff;     /* 指向DexCode结构的偏移 uleb128*/
};

// 代码概况
struct DexCode {
    u2 registersSize;   /* 使用的寄存器个数 */
    u2 insSize;         /* 参数个数 */
    u2 outsSize;        /* 调用其他方法时其它方法使用的寄存器个数，会在自己的调用栈申请，并压栈（猜测） */
    u2 triesSize;       /* Try/Catch个数 */
    u4 debugInfoOff;    /* 指向调试信息的偏移 */
    u4 insnsSize;       /* 指令集个数，以2字节为单位 */
    u2 insns[1];        /* 指令集 */
};

// 类的字段与方法概况
struct DexClassData {
    DexClassDataHeader header; /* 指定字段与方法的个数 */
    DexField* staticFields;    /* 静态字段，DexField结构 */
    DexField* instanceFields;  /* 实例字段，DexField结构 */
    DexMethod* directMethods;  /* 直接方法，DexMethod结构 */
    DexMethod* virtualMethods; /* 虚方法，DexMethod结构 */
};

//----------------数据区------------------------------
struct DexMapItem {
    u2 type;      /* kDexType开头的类型 */
    u2 unused;    /* 未使用，用于字节对齐 */
    u4 size;      /* 指定相应类型的个数 */
    u4 offset;    /* 指定相应类型的数据的文件偏移 */
};

//DEX map section
struct DexMapList {
    u4 size;               /* DexMapItem的个数，方便解析 */
    DexMapItem list[1];    /* 指向DexMapItem */
};

/* type字段为一个枚举常量，通过类型名称很容易判断它的具体类型。 */
/* map item type codes */
enum {
    kDexTypeHeaderItem = 0x0000,
    kDexTypeStringIdItem = 0x0001,
    kDexTypeTypeIdItem = 0x0002,
    kDexTypeProtoIdItem = 0x0003,
    kDexTypeFieldIdItem = 0x0004,
    kDexTypeMethodIdItem = 0x0005,
    kDexTypeClassDefItem = 0x0006,
    kDexTypeMapList = 0x1000,
    kDexTypeTypeList = 0x1001,
    kDexTypeAnnotationSetRefList = 0x1002,
    kDexTypeAnnotationSetItem = 0x1003,
    kDexTypeClassDataItem = 0x2000,
    kDexTypeCodeItem = 0x2001,
    kDexTypeStringDataItem = 0x2002,
    kDexTypeDebugInfoItem = 0x2003,
    kDexTypeAnnotationItem = 0x2004,
    kDexTypeEncodedArrayItem = 0x2005,
    kDexTypeAnnotationsDirectoryItem = 0x2006,
};



class DexParse {
public:
    // 解码字符串工具函数
    std::string parseString(const std::vector<uint8_t>& byteArray, size_t stringDataOff) {
        size_t offset = stringDataOff;
        unsigned int length = readUnsignedLeb128(byteArray, offset);
        return decodeString(byteArray, offset, length);
    }
    // 模拟 ULEB128 解码
    unsigned int readUnsignedLeb128(const std::vector<uint8_t>& byteArray, size_t& offset) {
        unsigned int result = 0;
        int shift = 0;
        uint8_t byteRead;

        do {
            byteRead = byteArray[offset++];
            result |= (byteRead & 0x7F) << shift;
            shift += 7;
        } while (byteRead & 0x80);  // 继续读取，直到字节的高位为 0

        return result;
    }
    // 模拟 MUTF-8 解码
    std::string decodeString(const std::vector<uint8_t>& byteArray, size_t offset, size_t length) {
        std::string result;
        size_t index = offset;

        while (index < offset + length) {
            uint8_t byte = byteArray[index++];

            if (byte <= 0x7F) {
                // 单字节字符（ASCII）
                result.push_back(static_cast<char>(byte));
            }
            else if ((byte >> 5) == 0x06) {
                // 两字节字符（MUTF-8的第2类字符）
                uint8_t byte2 = byteArray[index++];
                result.push_back(static_cast<char>(((byte & 0x1F) << 6) | (byte2 & 0x3F)));
            }
            else if ((byte >> 4) == 0x0E) {
                // 三字节字符（MUTF-8的第3类字符）
                uint8_t byte2 = byteArray[index++];
                uint8_t byte3 = byteArray[index++];
                // 合并成一个 Unicode 字符
                uint32_t character = ((byte & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
                result.push_back(static_cast<char>(character));  // 正确处理 3 字节的字符
            }
        }

        return result;
    }
    // 辅助函数：解析并打印字符串
    void processAndPrintString(const std::vector<uint8_t>& byteArray, size_t stringDataOff, int index) {
        // 解析字符串
        std::string decodedString = parseString(byteArray, stringDataOff);

        // 保存解码后的字符串到全局变量
        decodedStrings.push_back(decodedString);

        for (char ch : decodedString) {
            if (ch == '\n') {
                printf("\\n");
            }
            else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
    void ParseDexHeader(FILE* fp);
    void ParseStringIds(FILE* fp);
    void ParseTypeIds(FILE* fp);
    void ParseProtoIds(FILE* fp);
    void ParseDexFieldIds(FILE* fp);
    void ParseDexMethodId(FILE* fp);
    void ParseDexClass(FILE* fp);
private:
    std::vector<std::string> decodedStrings;
    std::vector<std::string> typeStrings;
    std::vector<std::string> protoStrings;
};
