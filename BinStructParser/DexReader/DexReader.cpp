#include"DexReader.h"

void DexParse::ParseDexHeader(FILE* fp) {
    // 为 dex_hdr 分配内存
    DexHeader* dex_hdr = new DexHeader();
    // 移动文件指针到文件开头
    fseek(fp, 0, SEEK_SET);
    // 从文件中读取 DexHeader 内容到动态分配的内存中
    size_t bytesRead = fread(dex_hdr, sizeof(DexHeader), 1, fp);
    if (bytesRead != 1) {
        perror("Failed to read DexHeader");
        delete dex_hdr;  // 确保释放内存
        dex_hdr = nullptr;
        return;
    }
    // 输出文件标识符
    printf("文件标识符：");
    for (int i = 0; i < 8; i++) {
        if (dex_hdr->magic[i] == '\n') {
            printf("\\n");
        }
        else {
            printf("%c", dex_hdr->magic[i]);
        }
    }
    printf("\n");
    printf("校验码：%x\n", dex_hdr->checksum);
    printf("SHA-1 签名：");
    for (int i = 0; i < kSHA1DigestLen; i++) {
        printf("%02x", dex_hdr->signature[i]);
    }
    printf("\n");
    printf("文件大小：%u 字节\n", dex_hdr->fileSize);
    printf("文件头的大小：%u 字节\n", dex_hdr->headerSize);
    printf("字节序标记：0x%08x", dex_hdr->endianTag);
    // 判断字节序类型
    if (dex_hdr->endianTag == 0x12345678) {
        printf("[大端序（Big-Endianness）]\n");
    }
    else if (dex_hdr->endianTag == 0x78563412) {
        printf("[小端序（Little-Endianness）]\n");
    }
    else {
        printf("字节序标记未知\n");
    }
    printf("链接段大小：%u 字节\n", dex_hdr->linkSize);
    printf("链接段偏移位置：0x%08x\n", dex_hdr->linkOff);
    printf("map偏移位置：0x%08x\n", dex_hdr->mapOff);
    printf("字符串数量：%u\n", dex_hdr->stringIdsSize);
    printf("字符串表偏移：0x%08x\n", dex_hdr->stringIdsOff);
    printf("类型标识符列表中的元素数量：%u\n", dex_hdr->typeIdsSize);
    printf("类型标识符表偏移：0x%08x\n", dex_hdr->typeIdsOff);
    printf("原型（方法）标识符列表中的元素数量：%u\n", dex_hdr->protoIdsSize);
    printf("原型（方法）标识符表偏移：0x%08x\n", dex_hdr->protoIdsOff);
    printf("字段标识符列表中的元素数量：%u\n", dex_hdr->fieldIdsSize);
    printf("字段标识符表偏移：0x%08x\n", dex_hdr->fieldIdsOff);
    printf("方法标识符列表中的元素数量：%u\n", dex_hdr->methodIdsSize);
    printf("方法标识符表偏移：0x%08x\n", dex_hdr->methodIdsOff);
    printf("类定义列表中的元素数量：%u\n", dex_hdr->classDefsSize);
    printf("类定义列表偏移：0x%08x\n", dex_hdr->classDefsOff);
    printf("方法标识符列表中的元素数量：%u\n", dex_hdr->dataSize);
    printf("方法标识符表偏移：0x%08x\n", dex_hdr->dataOff);
    // 释放内存
    delete dex_hdr;
    dex_hdr = nullptr;
}

// 解析字符串 ID 区
void DexParse::ParseStringIds(FILE* fp) {
   
    //fseek(fp, 0, SEEK_END);
    //size_t fileSize = ftell(fp);
    //fseek(fp, 0, SEEK_SET);

    //// 为 dex_hdr 分配内存
    //DexHeader* dex_hdr = new DexHeader();
    //// 移动文件指针到文件开头
    //fseek(fp, 0, SEEK_SET);
    //// 从文件中读取 DexHeader 内容到动态分配的内存中
    //size_t bytesRead = fread(dex_hdr, sizeof(DexHeader), 1, fp);
    //if (bytesRead != 1) {
    //    perror("Failed to read DexHeader");
    //    delete dex_hdr;  // 确保释放内存
    //    dex_hdr = nullptr;
    //    return;
    //}
    //// 获取文件大小

    //// 读取整个文件到 byteArray
    //std::vector<uint8_t> byteArray(fileSize);
    //fread(byteArray.data(), 1, fileSize, fp);

    //// 读取字符串 ID 区
    //DexStringId stringtable[999];  // 由于字符串数量为 418，定义合适大小的数组
    //fseek(fp, dex_hdr->stringIdsOff, SEEK_SET);  // 定位到 stringIds 区段
    //fread(stringtable, sizeof(DexStringId), dex_hdr->stringIdsSize, fp);  // 读取字符串 ID 信息
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 读取整个文件到 byteArray
    std::vector<uint8_t> byteArray(fileSize);
    if (fread(byteArray.data(), 1, fileSize, fp) != fileSize) {
        perror("Failed to read file");
        fclose(fp);
        return;
    }

    // 使用智能指针管理 dex_hdr 内存
    auto dex_hdr = std::make_unique<DexHeader>();

    // 从 byteArray 读取 DexHeader 内容
    memcpy(dex_hdr.get(), byteArray.data(), sizeof(DexHeader));

    // 读取字符串 ID 区
    size_t stringIdsOffset = dex_hdr->stringIdsOff;
    size_t stringIdsSize = dex_hdr->stringIdsSize;

    // 读取字符串表到 stringtable 数组
    std::vector<DexStringId> stringtable(stringIdsSize);
    memcpy(stringtable.data(), byteArray.data() + stringIdsOffset, sizeof(DexStringId) * stringIdsSize);

    printf("=========================Strings==========================================\n");
    // 遍历并打印所有字符串
    for (int i = 0; i < dex_hdr->stringIdsSize; i++) {
        // 调用辅助函数，处理并打印每个字符串
        printf("String %d :", i);
        processAndPrintString(byteArray, stringtable[i].stringDataOff, i);
    }
}

// 解析typeID 区
void DexParse::ParseTypeIds(FILE* fp) {
   
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

     // 读取整个文件到 byteArray
    std::vector<uint8_t> byteArray(fileSize);
    fread(byteArray.data(), 1, fileSize, fp);

    // 使用智能指针管理 dex_hdr 内存
    auto dex_hdr = std::make_unique<DexHeader>();

    // 从 byteArray 读取 DexHeader 内容
    memcpy(dex_hdr.get(), byteArray.data(), sizeof(DexHeader));

    // 读取type id表到 typetable 数组
    std::vector<DexTypeId> Typetable(dex_hdr->typeIdsSize);
    memcpy(Typetable.data(), byteArray.data() + dex_hdr->typeIdsOff, sizeof(DexTypeId) * dex_hdr->typeIdsSize);
    printf("============================Types============================\n");

    // 遍历并打印所有类型的字符串
    for (int i = 0; i < dex_hdr->typeIdsSize; i++) {
        printf("Type %d :", i);
        // 获取每个类型对应的字符串偏移（通过 descriptorIdx 获取对应的 DexStringId）
        uint32_t stringIdx = Typetable[i].descriptorIdx;
        // 获取对应的 DexStringId
        if (stringIdx < decodedStrings.size()) {
            std::string decodedString = decodedStrings[stringIdx];
            typeStrings.push_back(decodedString);
            printf("%s\n",decodedString.c_str());
        }
        else {
            printf("Invalid stringIdx: %u\n", stringIdx);
        }
    }
}


void DexParse::ParseProtoIds(FILE* fp) {
  
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // 读取整个文件到 byteArray
    std::vector<uint8_t> byteArray(fileSize);
    fread(byteArray.data(), 1, fileSize, fp);

    // 使用智能指针管理 dex_hdr 内存
    auto dex_hdr = std::make_unique<DexHeader>();
    // 从 byteArray 读取 DexHeader 内容
    memcpy(dex_hdr.get(), byteArray.data(), sizeof(DexHeader));


    // 读取原型 ID 区
    //std::vector<DexProtoId> Prototable(dex_hdr->protoIdsSize);  // 动态分配类型表
    //fseek(fp, dex_hdr->protoIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    //fread(Prototable.data(), sizeof(DexProtoId), dex_hdr->protoIdsSize, fp);  // 读取类型 ID 信息
    // 读取type id表到 typetable 数组
    std::vector<DexProtoId> Prototable(dex_hdr->protoIdsSize);
    memcpy(Prototable.data(), byteArray.data() + dex_hdr->protoIdsOff, sizeof(DexProtoId) * dex_hdr->protoIdsSize);

    printf("=======================Proto======================================\n");

    // 遍历并打印所有类型的字符串
    for (int i = 0; i < dex_hdr->protoIdsSize; i++) {
        printf("Proto %d :", i);
        // 获取每个类型对应的字符串偏移（通过 descriptorIdx 获取对应的 DexStringId）
        uint32_t shortyIdx = Prototable[i].shortyIdx;
        if (shortyIdx < decodedStrings.size()) {
            std::string decodedString1 = decodedStrings[shortyIdx];
            printf("%s", decodedString1.c_str());
        }
        else {
            printf("Invalid shortyIdx: %u", shortyIdx);
        }
        printf(":");

        // 获取每个类型对应的字符串偏移（通过 descriptorIdx 获取对应的 DexStringId）
        uint32_t returntersOff = Prototable[i].returnTypeIdx;
        if (returntersOff < typeStrings.size()) {
            std::string decodedString2 = typeStrings[returntersOff];
            typeStrings.push_back(decodedString2);
            printf("%s",decodedString2.c_str());
        }
        else {
            printf("Invalid returntersOff: %u", returntersOff);
        }

        printf("(");
        uint32_t parameters = Prototable[i].parametersOff;
        if (parameters == 0) {
            printf("proto %d has no parameters\n", i);
            continue; // 跳过没有参数的 proto
        }
        // 移动文件指针到 parameters 偏移位置
        fseek(fp, parameters, SEEK_SET);

        // 读取 TypeList 的 size
        DexTypeList typeListHeader;
        fread(&typeListHeader, sizeof(uint32_t), 1, fp); // 只读取 size

        // 检查 size 是否有效
        if (typeListHeader.size == 0) {
            printf("proto %d has no parameters\n", i);
            continue;
        }

        // 读取参数类型索引数组
        std::vector<DexTypeItem> typeIndices(typeListHeader.size);
        fread(typeIndices.data(), sizeof(DexTypeItem), typeListHeader.size, fp);

        // 遍历每个参数的类型索引
        for (int j = 0; j < typeListHeader.size; j++) {
            uint16_t typeIdx2 = typeIndices[j].typeIdx;
            if (typeIdx2 < typeStrings.size()) {
                std::string decodedString3 = typeStrings[typeIdx2];
                typeStrings.push_back(decodedString3);
                printf("%s", decodedString3.c_str());
                printf(",");
            }
            else {
                printf("Invalid returntersOff: %u", returntersOff);
            }
        }
        printf(")");

        printf("\n");
    }
}


void DexParse::ParseDexFieldIds(FILE* fp) {
   
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // 读取整个文件到 byteArray
    std::vector<uint8_t> byteArray(fileSize);
    fread(byteArray.data(), 1, fileSize, fp);

    // 使用智能指针管理 dex_hdr 内存
    auto dex_hdr = std::make_unique<DexHeader>();
    // 从 byteArray 读取 DexHeader 内容
    memcpy(dex_hdr.get(), byteArray.data(), sizeof(DexHeader));

    //// 读取字段标识符 ID 区
    //std::vector<DexFieldId> fieldtable(dex_hdr->fieldIdsSize);  // 动态分配类型表
    //fseek(fp, dex_hdr->fieldIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    //fread(fieldtable.data(), sizeof(DexFieldId), dex_hdr->fieldIdsSize, fp);  // 读取类型 ID 信息
    std::vector<DexFieldId> fieldtable(dex_hdr->fieldIdsSize);
    memcpy(fieldtable.data(), byteArray.data() + dex_hdr->fieldIdsOff, sizeof(DexFieldId) * dex_hdr->fieldIdsSize);

    printf("======================Field====================================\n");
    for (int i = 0; i < dex_hdr->fieldIdsSize; i++)
    {
        printf("Field %d: ", i);
        uint16_t typeIndex = fieldtable[i].typeIdx;
        if (typeIndex < typeStrings.size()) {
            std::string decodedString1 = typeStrings[typeIndex];
            printf("%s", decodedString1.c_str());
        }
        else {
            printf("Invalid shortyIdx: %u", typeIndex);
        }
        printf(" ");

        uint16_t typeIndex1 = fieldtable[i].classIdx;
        if (typeIndex1 < typeStrings.size()) {
            std::string decodedString2 = typeStrings[typeIndex];
            printf("%s", decodedString2.c_str());
        }
        else {
            printf("Invalid shortyIdx: %u", typeIndex1);
        }
        printf(".");
        uint32_t stringIdx3 = fieldtable[i].nameIdx;
        if (stringIdx3 < decodedStrings.size()) {
            std::string decodedString = decodedStrings[stringIdx3];
            typeStrings.push_back(decodedString);
            printf("%s",decodedString.c_str());
        }
        else {
            printf("Invalid stringIdx: %u\n", stringIdx3);
        }
       
        printf("\n");
    }
}


void DexParse::ParseDexMethodId(FILE* fp) {
    // 为 dex_hdr 分配内存
    DexHeader* dex_hdr = new DexHeader();
    // 移动文件指针到文件开头
    fseek(fp, 0, SEEK_SET);

    // 从文件中读取 DexHeader 内容到动态分配的内存中
    size_t bytesRead = fread(dex_hdr, sizeof(DexHeader), 1, fp);
    if (bytesRead != 1) {
        perror("Failed to read DexHeader");
        delete dex_hdr;  // 确保释放内存
        dex_hdr = nullptr;
        return;
    }
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 读取整个文件到 byteArray
    std::vector<uint8_t> byteArray(fileSize);
    fread(byteArray.data(), 1, fileSize, fp);

    // 读取字符串表
    std::vector<DexStringId> stringtable(dex_hdr->stringIdsSize);  // 动态分配字符串表
    fseek(fp, dex_hdr->stringIdsOff, SEEK_SET);  // 定位到字符串区段
    fread(stringtable.data(), sizeof(DexStringId), dex_hdr->stringIdsSize, fp);  // 读取字符串表数据

    // 读取类型 ID 区
    std::vector<DexTypeId> Typetable(dex_hdr->typeIdsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->typeIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(Typetable.data(), sizeof(DexTypeId), dex_hdr->typeIdsSize, fp);  // 读取类型 ID 信息

    // 读取原型 ID 区
    std::vector<DexProtoId> Prototable(dex_hdr->protoIdsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->protoIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(Prototable.data(), sizeof(DexProtoId), dex_hdr->protoIdsSize, fp);  // 读取类型 ID 信息

    // 读取方法标识符 ID 区
    std::vector<DexMethodId> methodtable(dex_hdr->methodIdsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->methodIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(methodtable.data(), sizeof(DexMethodId), dex_hdr->methodIdsSize, fp);  // 读取类型 ID 信息



    printf("======================Method=============================\n");
    for (int i = 0; i < dex_hdr->methodIdsSize; i++)
    {
        printf("method %d :", i);
        uint16_t protoIndex1 = methodtable[i].protoIdx;
        DexProtoId protoId = Prototable[protoIndex1];
        // 获取每个类型对应的字符串偏移（通过 descriptorIdx 获取对应的 DexStringId）
        uint32_t shortyIdx = protoId.shortyIdx;
        if (shortyIdx < decodedStrings.size()) {
            std::string decodedString = decodedStrings[shortyIdx];
            typeStrings.push_back(decodedString);
            printf("%s", decodedString.c_str());
        }
        else {
            printf("Invalid stringIdx: %u\n", shortyIdx);
        }
        printf(": ");

        uint16_t typeIndex = methodtable[i].classIdx;
        DexTypeId TypeId = Typetable[typeIndex];
        //printf("typeIndex: %d, descriptorIdx: %d\n", typeIndex, TypeId.descriptorIdx);  // 打印 descriptorIdx 的值
        uint32_t stringIdx = TypeId.descriptorIdx;
        if (stringIdx < decodedStrings.size()) {
            std::string decodedString1 = decodedStrings[stringIdx];
            typeStrings.push_back(decodedString1);
            printf("%s", decodedString1.c_str());
        }
        else {
            printf("Invalid stringIdx: %u\n", stringIdx);
        }

        printf(".");

        uint32_t stringIdx3 = methodtable[i].nameIdx;
        if (stringIdx3 < decodedStrings.size()) {
            std::string decodedString2 = decodedStrings[stringIdx3];
            typeStrings.push_back(decodedString2);
            printf("%s", decodedString2.c_str());
        }
        else {
            printf("Invalid stringIdx: %u\n", stringIdx3);
        }

        printf("\n");

    }
}


void DexParse::ParseDexClass(FILE* fp) {
    // 为 dex_hdr 分配内存
    DexHeader* dex_hdr = new DexHeader();
    // 移动文件指针到文件开头
    fseek(fp, 0, SEEK_SET);

    // 从文件中读取 DexHeader 内容到动态分配的内存中
    size_t bytesRead = fread(dex_hdr, sizeof(DexHeader), 1, fp);
    if (bytesRead != 1) {
        perror("Failed to read DexHeader");
        delete dex_hdr;  // 确保释放内存
        dex_hdr = nullptr;
        return;
    }
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 读取整个文件到 byteArray
    std::vector<uint8_t> byteArray(fileSize);
    fread(byteArray.data(), 1, fileSize, fp);

    // 读取字符串表
    std::vector<DexStringId> stringtable(dex_hdr->stringIdsSize);  // 动态分配字符串表
    fseek(fp, dex_hdr->stringIdsOff, SEEK_SET);  // 定位到字符串区段
    fread(stringtable.data(), sizeof(DexStringId), dex_hdr->stringIdsSize, fp);  // 读取字符串表数据

    // 读取type ID 区
    std::vector<DexTypeId> Typetable(dex_hdr->typeIdsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->typeIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(Typetable.data(), sizeof(DexTypeId), dex_hdr->typeIdsSize, fp);  // 读取类型 ID 信息

    // 读取proto ID 区
    std::vector<DexProtoId> Prototable(dex_hdr->protoIdsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->protoIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(Prototable.data(), sizeof(DexProtoId), dex_hdr->protoIdsSize, fp);  // 读取类型 ID 信息

    // 读取method ID 区
    std::vector<DexMethodId> methodtable(dex_hdr->methodIdsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->methodIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(methodtable.data(), sizeof(DexMethodId), dex_hdr->methodIdsSize, fp);  // 读取类型 ID 信息

    // 读取fild ID 区
    std::vector<DexFieldId> fieldtable(dex_hdr->fieldIdsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->fieldIdsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(fieldtable.data(), sizeof(DexFieldId), dex_hdr->fieldIdsSize, fp);  // 读取类型 ID 信息

    // 读取class ID 区
    std::vector<DexClassDef> classtable(dex_hdr->classDefsSize);  // 动态分配类型表
    fseek(fp, dex_hdr->classDefsOff, SEEK_SET);  // 定位到 typeIds 区段
    fread(classtable.data(), sizeof(DexClassDef), dex_hdr->classDefsSize, fp);  // 读取类型 ID 信息

    printf("===========================Class===============================\n");
    for (int i = 0; i < dex_hdr->classDefsSize; i++) 
    {
     /*   DexClassData* ClassData= new DexClassData();;
        fseek(fp, classtable[i].classDataOff, SEEK_SET);  
        fread(ClassData, sizeof(DexClassData), 1, fp);  

        DexClassDataHeader* classhdr = new DexClassDataHeader();
        fseek(fp, classtable[i].classDataOff, SEEK_SET);
        fread(classhdr, sizeof(DexClassDataHeader), 1, fp);
   
        int staticFieldCount = readUnsignedLeb128(classhdr, 0);
        int offset = staticFieldCount * sizeof(classhdr->staticFieldsSize);
        int instanceField = readUnsignedLeb128(classhdr,offset);
        int directMethods = readUnsignedLeb128(byteArray, classhdr->directMethodsSize);
        int virtualMethods = readUnsignedLeb128(byteArray, classhdr->virtualMethodsSize);*/
        printf("+++++++++++++++++++Class %d +++++++++++++++++++++++++++++++++++\n", i);
        size_t offset = classtable[i].classDataOff;
        if (offset >= byteArray.size()) {
            fprintf(stderr, "ClassData offset out of bounds\n");
            return;
        }

        DexClassDataHeader header;
        memcpy(&header, byteArray.data() + offset, sizeof(DexClassDataHeader));
        int staticFieldCount = readUnsignedLeb128(byteArray, offset);
        int instanceFieldCount = readUnsignedLeb128(byteArray, offset);  // offset 继续递增
        int directMethodCount = readUnsignedLeb128(byteArray, offset);  // offset 继续递增
        int virtualMethodCount = readUnsignedLeb128(byteArray, offset);  // offset 最终递增到下一个字段位置

        printf("staticFieldCount: %d \n", staticFieldCount);
        printf("instanceFieldCount: %d \n", instanceFieldCount);
        printf("directMethodCount: %d \n", directMethodCount);
        printf("virtualMethodCount: %d \n", virtualMethodCount);
        //printf("----------------\n");

        
        if (staticFieldCount > 0) {
            printf("---------------------static Fields=================================================> \n");
            while (staticFieldCount--) {
                int tmpLength = 0;
                int fieldIndx = readUnsignedLeb128(byteArray, offset);
                int accessFlags = readUnsignedLeb128(byteArray, offset);

                //----field----------
                uint16_t typeIndex = fieldtable[fieldIndx].typeIdx;
                DexTypeId TypeId = Typetable[typeIndex];
                uint32_t stringIdx = TypeId.descriptorIdx;
                if (stringIdx >= dex_hdr->stringIdsSize) {
                    printf("  Invalid descriptorIdx: %d\n", stringIdx);
                    continue;
                }
                DexStringId stringId = stringtable[stringIdx];
                size_t stringOffset = stringId.stringDataOff;
                if (stringOffset >= byteArray.size()) {
                    printf("  Invalid stringDataOff: %zu\n", stringOffset);
                    continue;
                }
                // 解码字符串
                size_t offset0 = stringOffset;
                unsigned int length = readUnsignedLeb128(byteArray, offset0);
                std::string decodedString = decodeString(byteArray, offset0, length);

                // 打印解码后的字符串
                for (char ch : decodedString) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else if (ch == ';') {
                        continue;
                    }
                    else {
                        printf("%c", ch);
                    }
                }

                printf(" ");
                uint16_t typeIndex1 = fieldtable[i].classIdx;
                DexTypeId TypeId1 = Typetable[typeIndex1];
                uint32_t stringIdx1 = TypeId1.descriptorIdx;
                if (stringIdx1 >= dex_hdr->stringIdsSize) {
                    printf("  Invalid descriptorIdx: %d\n", stringIdx);
                    continue;
                }
                DexStringId stringId1 = stringtable[stringIdx1];
                size_t stringOffset1 = stringId1.stringDataOff;
                if (stringOffset1 >= byteArray.size()) {
                    printf("  Invalid stringDataOff: %zu\n", stringOffset1);
                    continue;
                }
                // 解码字符串
                size_t offset1 = stringOffset1;
                unsigned int length1 = readUnsignedLeb128(byteArray, offset1);
                std::string decodedString1 = decodeString(byteArray, offset1, length1);
                // 打印解码后的字符串
                for (char ch : decodedString1) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else if (ch == ';') {
                        continue;
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf(".");

                uint32_t stringIdx3 = fieldtable[i].nameIdx;
                if (stringIdx1 >= dex_hdr->stringIdsSize) {
                    printf("  Invalid descriptorIdx: %d\n", stringIdx);
                    continue;
                }
                DexStringId stringId3 = stringtable[stringIdx3];
                size_t stringOffset3 = stringId3.stringDataOff;
                if (stringOffset3 >= byteArray.size()) {
                    printf("  Invalid stringDataOff: %zu\n", stringOffset);
                    continue;
                }
                // 解码字符串
                size_t offset3 = stringOffset3;
                unsigned int length3 = readUnsignedLeb128(byteArray, offset3);
                std::string decodedString3 = decodeString(byteArray, offset3, length3);
                // 打印解码后的字符串
                for (char ch : decodedString3) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else if (ch == ';') {
                        continue;
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf("\n");
                printf(" accflag=0x%X\n", accessFlags);
            }
        }

        if (instanceFieldCount > 0) {
            printf("----------------------instance Field========================================>\n");
                while (instanceFieldCount--) {
                    int instanceIndx = readUnsignedLeb128(byteArray, offset);
                    int accessFlags = readUnsignedLeb128(byteArray, offset);
                    //----instance----------
                    uint16_t typeIndex = fieldtable[instanceIndx].typeIdx;
                    DexTypeId TypeId = Typetable[typeIndex];
                    uint32_t stringIdx = TypeId.descriptorIdx;
                    if (stringIdx >= dex_hdr->stringIdsSize) {
                        printf("  Invalid descriptorIdx: %d\n", stringIdx);
                        continue;
                    }
                    DexStringId stringId = stringtable[stringIdx];
                    size_t stringOffset = stringId.stringDataOff;
                    if (stringOffset >= byteArray.size()) {
                        printf("  Invalid stringDataOff: %zu\n", stringOffset);
                        continue;
                    }
                    // 解码字符串
                    size_t offset0 = stringOffset;
                    unsigned int length = readUnsignedLeb128(byteArray, offset0);
                    std::string decodedString = decodeString(byteArray, offset0, length);

                    // 打印解码后的字符串
                    for (char ch : decodedString) {
                        if (ch == '\n') {
                            printf("\\n");
                        }
                        else if (ch == ';') {
                            continue;
                        }
                        else {
                            printf("%c", ch);
                        }
                    }
                    printf(" ");
                    uint16_t typeIndex1 = fieldtable[i].classIdx;
                    DexTypeId TypeId1 = Typetable[typeIndex1];
                    uint32_t stringIdx1 = TypeId1.descriptorIdx;
                    if (stringIdx1 >= dex_hdr->stringIdsSize) {
                        printf("  Invalid descriptorIdx: %d\n", stringIdx);
                        continue;
                    }
                    DexStringId stringId1 = stringtable[stringIdx1];
                    size_t stringOffset1 = stringId1.stringDataOff;
                    if (stringOffset1 >= byteArray.size()) {
                        printf("  Invalid stringDataOff: %zu\n", stringOffset1);
                        continue;
                    }
                    // 解码字符串
                    size_t offset1 = stringOffset1;
                    unsigned int length1 = readUnsignedLeb128(byteArray, offset1);
                    std::string decodedString1 = decodeString(byteArray, offset1, length1);
                    // 打印解码后的字符串
                    for (char ch : decodedString1) {
                        if (ch == '\n') {
                            printf("\\n");
                        }
                        else if (ch == ';') {
                            continue;
                        }
                        else {
                            printf("%c", ch);
                        }
                    }
                    printf(".");

                    uint32_t stringIdx3 = fieldtable[i].nameIdx;
                    if (stringIdx1 >= dex_hdr->stringIdsSize) {
                        printf("  Invalid descriptorIdx: %d\n", stringIdx);
                        continue;
                    }
                    DexStringId stringId3 = stringtable[stringIdx3];
                    size_t stringOffset3 = stringId3.stringDataOff;
                    if (stringOffset3 >= byteArray.size()) {
                        printf("  Invalid stringDataOff: %zu\n", stringOffset);
                        continue;
                    }
                    // 解码字符串
                    size_t offset3 = stringOffset3;
                    unsigned int length3 = readUnsignedLeb128(byteArray, offset3);
                    std::string decodedString3 = decodeString(byteArray, offset3, length3);
                    // 打印解码后的字符串
                    for (char ch : decodedString3) {
                        if (ch == '\n') {
                            printf("\\n");
                        }
                        else if (ch == ';') {
                            continue;
                        }
                        else {
                            printf("%c", ch);
                        }
                    }
                    printf("\n");
                    printf(" accflag=0x%X\n", accessFlags);
               }
                
            
        }

        if (directMethodCount > 0) {
            printf("-----------------direct Method===================================> \n");
            while (directMethodCount--) {
                int directMethodIndx = readUnsignedLeb128(byteArray, offset);
                int accessFlags = readUnsignedLeb128(byteArray, offset);
                int codeOff = readUnsignedLeb128(byteArray, offset);
                //----------directMethod
                uint16_t protoIndex1 = methodtable[directMethodIndx].protoIdx;
                DexProtoId protoId = Prototable[protoIndex1];
                // 获取每个类型对应的字符串偏移（通过 descriptorIdx 获取对应的 DexStringId）
                uint32_t shortyIdx = protoId.shortyIdx;
                // 获取对应的 DexStringId
                DexStringId stringId = stringtable[shortyIdx];
                // 获取字符串的偏移量
                size_t stringOffset = stringId.stringDataOff;
                // 读取该字符串的 ULEB128 解码的长度
                size_t offset0 = stringOffset;
                unsigned int length = readUnsignedLeb128(byteArray, offset0);
                // 解码字符串（MUTF-8）
                std::string decodedString = decodeString(byteArray, offset0, length);
                // 打印解码后的字符串
                for (char ch : decodedString) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf(": ");

                uint16_t typeIndex = methodtable[i].classIdx;
                // 检查类型索引是否有效
                if (typeIndex >= dex_hdr->typeIdsSize) {
                    printf("Invalid typeIndex: %d\n", typeIndex);
                    continue;
                }
                DexTypeId TypeId = Typetable[typeIndex];
                //printf("typeIndex: %d, descriptorIdx: %d\n", typeIndex, TypeId.descriptorIdx);  // 打印 descriptorIdx 的值
                uint32_t stringIdx = TypeId.descriptorIdx;
                if (stringIdx >= dex_hdr->stringIdsSize) {
                    printf("  Invalid descriptorIdx: %d\n", stringIdx);
                    continue;
                }
                DexStringId stringId2 = stringtable[stringIdx];
                size_t stringOffset2 = stringId2.stringDataOff;
                if (stringOffset >= byteArray.size()) {
                    printf("  Invalid stringDataOff: %zu\n", stringOffset);
                    continue;
                }
                // 解码字符串
                size_t offset2 = stringOffset2;
                unsigned int length2 = readUnsignedLeb128(byteArray, offset2);
                std::string decodedString2 = decodeString(byteArray, offset, length2);

                // 打印解码后的字符串
                for (char ch : decodedString2) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else if (ch == ';') {
                        continue;
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf(".");

                uint32_t stringIdx3 = methodtable[i].nameIdx;
                if (stringIdx3 >= dex_hdr->stringIdsSize) {
                    printf("  Invalid descriptorIdx: %d\n", stringIdx3);
                    continue;
                }
                DexStringId stringId3 = stringtable[stringIdx3];
                size_t stringOffset3 = stringId3.stringDataOff;
                if (stringOffset3 >= byteArray.size()) {
                    printf("  Invalid stringDataOff: %zu\n", stringOffset);
                    continue;
                }
                // 解码字符串
                size_t offset3 = stringOffset3;
                unsigned int length3 = readUnsignedLeb128(byteArray, offset3);
                std::string decodedString3 = decodeString(byteArray, offset3, length3);

                // 打印解码后的字符串
                for (char ch : decodedString3) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else if (ch == ';') {
                        continue;
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf("\n");
                printf(" accflag=0x%X\n", accessFlags);

            
            }
        }
        
        if (virtualMethodCount > 0) {
            printf("----------------virtual Method=============================> \n");
            while (virtualMethodCount--) {
                int virtualMethodIndx = readUnsignedLeb128(byteArray, offset);
                int accessFlags = readUnsignedLeb128(byteArray, offset);
                int codeOff = readUnsignedLeb128(byteArray, offset);
                //----------directMethod
                uint16_t protoIndex1 = methodtable[virtualMethodIndx].protoIdx;
                DexProtoId protoId = Prototable[protoIndex1];
                // 获取每个类型对应的字符串偏移（通过 descriptorIdx 获取对应的 DexStringId）
                uint32_t shortyIdx = protoId.shortyIdx;
                // 获取对应的 DexStringId
                DexStringId stringId = stringtable[shortyIdx];
                // 获取字符串的偏移量
                size_t stringOffset = stringId.stringDataOff;
                // 读取该字符串的 ULEB128 解码的长度
                size_t offset0 = stringOffset;
                unsigned int length = readUnsignedLeb128(byteArray, offset0);
                // 解码字符串（MUTF-8）
                std::string decodedString = decodeString(byteArray, offset0, length);
                // 打印解码后的字符串
                for (char ch : decodedString) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf(": ");

                uint16_t typeIndex = methodtable[i].classIdx;
                // 检查类型索引是否有效
                if (typeIndex >= dex_hdr->typeIdsSize) {
                    printf("Invalid typeIndex: %d\n", typeIndex);
                    continue;
                }
                DexTypeId TypeId = Typetable[typeIndex];
                //printf("typeIndex: %d, descriptorIdx: %d\n", typeIndex, TypeId.descriptorIdx);  // 打印 descriptorIdx 的值
                uint32_t stringIdx = TypeId.descriptorIdx;
                if (stringIdx >= dex_hdr->stringIdsSize) {
                    printf("  Invalid descriptorIdx: %d\n", stringIdx);
                    continue;
                }
                DexStringId stringId2 = stringtable[stringIdx];
                size_t stringOffset2 = stringId2.stringDataOff;
                if (stringOffset >= byteArray.size()) {
                    printf("  Invalid stringDataOff: %zu\n", stringOffset);
                    continue;
                }
                // 解码字符串
                size_t offset2 = stringOffset2;
                unsigned int length2 = readUnsignedLeb128(byteArray, offset2);
                std::string decodedString2 = decodeString(byteArray, offset, length2);

                // 打印解码后的字符串
                for (char ch : decodedString2) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else if (ch == ';') {
                        continue;
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf(".");

                uint32_t stringIdx3 = methodtable[i].nameIdx;
                if (stringIdx3 >= dex_hdr->stringIdsSize) {
                    printf("  Invalid descriptorIdx: %d\n", stringIdx3);
                    continue;
                }
                DexStringId stringId3 = stringtable[stringIdx3];
                size_t stringOffset3 = stringId3.stringDataOff;
                if (stringOffset3 >= byteArray.size()) {
                    printf("  Invalid stringDataOff: %zu\n", stringOffset);
                    continue;
                }
                // 解码字符串
                size_t offset3 = stringOffset3;
                unsigned int length3 = readUnsignedLeb128(byteArray, offset3);
                std::string decodedString3 = decodeString(byteArray, offset3, length3);

                // 打印解码后的字符串
                for (char ch : decodedString3) {
                    if (ch == '\n') {
                        printf("\\n");
                    }
                    else if (ch == ';') {
                        continue;
                    }
                    else {
                        printf("%c", ch);
                    }
                }
                printf("\n");
                printf(" accflag=0x%X\n", accessFlags);


            }
        }
    }

}