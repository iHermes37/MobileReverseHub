#include <iostream>
#include <vector>
#include <string>

// 模拟 ULEB128 编码
void writeUnsignedLeb128(std::vector<uint8_t>& byteArray, unsigned int value) {
    while (value > 0x7F) {
        byteArray.push_back(static_cast<uint8_t>(value | 0x80));  // 设置高位为 1，表示还有更多字节
        value >>= 7;
    }
    byteArray.push_back(static_cast<uint8_t>(value));  // 最后一个字节的高位为 0
}

// 模拟 MUTF-8 编码
void encodeString(std::vector<uint8_t>& byteArray, const std::string& str) {
    for (char ch : str) {
        unsigned char byte = static_cast<unsigned char>(ch);
        if (byte <= 0x7F) {
            byteArray.push_back(byte);  // 单字节字符
        }
        else if ((byte >> 5) == 0x06) {
            // 两字节字符（MUTF-8的第2类字符）
            byteArray.push_back(0xC0 | (byte >> 6));
            byteArray.push_back(0x80 | (byte & 0x3F));
        }
        else if ((byte >> 4) == 0x0E) {
            // 三字节字符（MUTF-8的第3类字符）
            byteArray.push_back(0xE0 | (byte >> 12));
            byteArray.push_back(0x80 | ((byte >> 6) & 0x3F));
            byteArray.push_back(0x80 | (byte & 0x3F));
        }
    }
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

// 修正 MUTF-8 解码
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

//int main() {
//    // 1. 用户输入字符串
//    std::string inputStr;
//    std::cout << "请输入要编码的字符串: ";
//    std::getline(std::cin, inputStr);  // 用户输入字符串
//
//    // 2. 编码过程
//    std::vector<uint8_t> byteArray;
//    // ULEB128 编码长度
//    writeUnsignedLeb128(byteArray, inputStr.size());
//    // MUTF-8 编码字符串
//    encodeString(byteArray, inputStr);
//
//    // 打印编码后的字节流
//    std::cout << "编码后的字节流: ";
//    for (uint8_t byte : byteArray) {
//        std::cout << std::hex << (int)byte << " ";
//    }
//    std::cout << std::dec << std::endl;  // 输出后重置十进制
//
//    // 3. 解码过程
//    size_t offset = 0;
//    unsigned int length = readUnsignedLeb128(byteArray, offset);  // 获取字符串长度
//    std::cout << "ULEB128 解码后的字符串长度: " << length << std::endl; // 打印 ULEB128 解码后的字符串长度
//    std::string decodedString = decodeString(byteArray, offset, length);  // 解码字符串
//
//    // 打印解码后的字符个数
//    std::cout << "解码后的字符个数: " << decodedString.size() << std::endl;
//
//    // 4. 输出解码结果
//    std::cout << "解码后的字符串: " << decodedString << std::endl;
//
//    return 0;
//}
