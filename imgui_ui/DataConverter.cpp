/**
 * @file DataConverter.cpp
 * @brief 数据转换工具实现
 */

#include "DataConverter.h"
#include "core/AppState.h"  // 包含EncodingType枚举定义
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <windows.h>  // Windows编码转换API

std::string DataConverter::BytesToHexString(const unsigned char* data, int length, bool addSpaces) {
    if (!data || length <= 0) {
        return "";
    }

    std::ostringstream oss;
    for (int i = 0; i < length; i++) {
        oss << ByteToHex(data[i]);
        if (addSpaces && i < length - 1) {
            oss << " ";
        }
    }

    return oss.str();
}

bool DataConverter::HexStringToBytes(const std::string& hexStr, std::vector<unsigned char>& outData) {
    outData.clear();

    std::string cleanHex;
    // 移除非十六进制字符（空格等）
    for (char ch : hexStr) {
        if (std::isxdigit(ch)) {
            cleanHex += ch;
        }
    }

    // HEX字符串长度必须为偶数
    if (cleanHex.length() % 2 != 0) {
        return false;
    }

    // 两两解析为字节
    for (size_t i = 0; i < cleanHex.length(); i += 2) {
        unsigned char high, low;
        if (!HexCharToValue(cleanHex[i], high) || !HexCharToValue(cleanHex[i + 1], low)) {
            outData.clear();
            return false;
        }
        outData.push_back((high << 4) | low);
    }

    return true;
}

std::string DataConverter::BytesToAsciiString(const unsigned char* data, int length, bool replaceNonPrintable) {
    if (!data || length <= 0) {
        return "";
    }

    std::string result;
    result.reserve(length);

    for (int i = 0; i < length; i++) {
        if (replaceNonPrintable && !IsPrintable(data[i])) {
            result += '.';
        } else {
            result += static_cast<char>(data[i]);
        }
    }

    return result;
}

std::string DataConverter::FormatHexDump(const unsigned char* data, int length) {
    if (!data || length <= 0) {
        return "";
    }

    std::ostringstream oss;
    const int bytesPerLine = 16;

    for (int offset = 0; offset < length; offset += bytesPerLine) {
        // 打印偏移量
        oss << std::setw(4) << std::setfill('0') << std::hex << offset << ": ";

        // HEX部分
        int lineBytes = std::min(bytesPerLine, length - offset);
        for (int i = 0; i < bytesPerLine; i++) {
            if (i < lineBytes) {
                oss << ByteToHex(data[offset + i]) << " ";
            } else {
                oss << "   ";
            }
        }

        // ASCII表示
        oss << " ";
        for (int i = 0; i < lineBytes; i++) {
            unsigned char ch = data[offset + i];
            oss << (IsPrintable(ch) ? static_cast<char>(ch) : '.');
        }

        oss << "\n";
    }

    return oss.str();
}

bool DataConverter::IsPrintable(unsigned char ch) {
    return (ch >= 32 && ch <= 126);  // 可打印ASCII字符范围
}

std::string DataConverter::ByteToHex(unsigned char byte) {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(byte);
    return oss.str();
}

bool DataConverter::HexCharToValue(char ch, unsigned char& value) {
    if (ch >= '0' && ch <= '9') {
        value = ch - '0';
        return true;
    } else if (ch >= 'A' && ch <= 'F') {
        value = ch - 'A' + 10;
        return true;
    } else if (ch >= 'a' && ch <= 'f') {
        value = ch - 'a' + 10;
        return true;
    }
    return false;
}

// ========================================
// 编码转换函数实现
// ========================================

std::string DataConverter::ConvertToUTF8(const unsigned char* data, int length, EncodingType encoding) {
    if (!data || length <= 0) {
        return "";
    }

    // 如果已经是UTF-8或ASCII，直接返回
    if (encoding == EncodingType::UTF8 || encoding == EncodingType::ASCII) {
        return std::string(reinterpret_cast<const char*>(data), length);
    }

    // 确定源编码的代码页
    UINT sourceCodePage = 0;
    switch (encoding) {
        case EncodingType::GBK:
            sourceCodePage = 936;  // GBK/GB2312代码页
            break;
        default:
            return std::string(reinterpret_cast<const char*>(data), length);
    }

    // 步骤1：源编码 → Unicode (UTF-16)
    int wlen = MultiByteToWideChar(sourceCodePage, 0,
                                    reinterpret_cast<const char*>(data), length,
                                    NULL, 0);
    if (wlen <= 0) {
        // 转换失败，返回原始数据
        return std::string(reinterpret_cast<const char*>(data), length);
    }

    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(sourceCodePage, 0,
                       reinterpret_cast<const char*>(data), length,
                       &wstr[0], wlen);

    // 步骤2：Unicode (UTF-16) → UTF-8
    int utf8len = WideCharToMultiByte(CP_UTF8, 0,
                                      wstr.c_str(), wlen,
                                      NULL, 0, NULL, NULL);
    if (utf8len <= 0) {
        return std::string(reinterpret_cast<const char*>(data), length);
    }

    std::string utf8str(utf8len, 0);
    WideCharToMultiByte(CP_UTF8, 0,
                        wstr.c_str(), wlen,
                        &utf8str[0], utf8len, NULL, NULL);

    return utf8str;
}

bool DataConverter::ConvertFromUTF8(const std::string& utf8Str, EncodingType encoding, std::vector<unsigned char>& outData) {
    outData.clear();

    if (utf8Str.empty()) {
        return true;
    }

    // 如果目标是UTF-8或ASCII，直接复制
    if (encoding == EncodingType::UTF8 || encoding == EncodingType::ASCII) {
        outData.assign(utf8Str.begin(), utf8Str.end());
        return true;
    }

    // 确定目标编码的代码页
    UINT targetCodePage = 0;
    switch (encoding) {
        case EncodingType::GBK:
            targetCodePage = 936;  // GBK/GB2312代码页
            break;
        default:
            outData.assign(utf8Str.begin(), utf8Str.end());
            return true;
    }

    // 步骤1：UTF-8 → Unicode (UTF-16)
    int wlen = MultiByteToWideChar(CP_UTF8, 0,
                                    utf8Str.c_str(), utf8Str.length(),
                                    NULL, 0);
    if (wlen <= 0) {
        return false;
    }

    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0,
                       utf8Str.c_str(), utf8Str.length(),
                       &wstr[0], wlen);

    // 步骤2：Unicode (UTF-16) → 目标编码
    int outlen = WideCharToMultiByte(targetCodePage, 0,
                                     wstr.c_str(), wlen,
                                     NULL, 0, NULL, NULL);
    if (outlen <= 0) {
        return false;
    }

    std::string tempStr(outlen, 0);
    WideCharToMultiByte(targetCodePage, 0,
                        wstr.c_str(), wlen,
                        &tempStr[0], outlen, NULL, NULL);

    outData.assign(tempStr.begin(), tempStr.end());
    return true;
}
