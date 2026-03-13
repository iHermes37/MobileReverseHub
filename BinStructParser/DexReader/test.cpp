#include"DexReader.h"

int main() {
    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, "D:\\1\\classes.dex", "rb");
    if (err != 0) {
        perror("Failed to open file");
        return 1;
    }
    if (fp == NULL) {
        perror("Failed to open file");
        return 1;
    }
    DexParse dexreader;
    dexreader.ParseDexHeader(fp);
    dexreader.ParseStringIds(fp);
    dexreader.ParseTypeIds(fp);
    dexreader.ParseProtoIds(fp);
    dexreader.ParseDexFieldIds(fp);
    dexreader.ParseDexMethodId(fp);
    dexreader.ParseDexClass(fp);
}