/**
 * @file DataConverter.h
 * @brief 数据转换工具 - HEX/ASCII转换
 * @author AI Assistant
 * @date 2025
 */

#ifndef DATA_CONVERTER_H
#define DATA_CONVERTER_H

#include <string>
#include <vector>

// 前置声明编码类型（避免循环依赖）
enum class EncodingType;

/**
 * @brief 数据转换工具类
 */
class DataConverter {
public:
    /**
     * @brief 将字节数组转换为HEX字符串
     * @param data 字节数组
     * @param length 数据长度
     * @param addSpaces 是否在字节之间添加空格
     * @return HEX字符串 (例如: "48 65 6C 6C 6F")
     */
    static std::string BytesToHexString(const unsigned char* data, int length, bool addSpaces = true);

    /**
     * @brief 将HEX字符串转换为字节数组
     * @param hexStr HEX字符串 (例如: "48656C6C6F" 或 "48 65 6C 6C 6F")
     * @param outData 输出字节数组
     * @return 成功返回true，失败返回false
     */
    static bool HexStringToBytes(const std::string& hexStr, std::vector<unsigned char>& outData);

    /**
     * @brief 将字节数组转换为ASCII字符串
     * @param data 字节数组
     * @param length 数据长度
     * @param replaceNonPrintable 是否将不可打印字符替换为'.'
     * @return ASCII字符串
     */
    static std::string BytesToAsciiString(const unsigned char* data, int length, bool replaceNonPrintable = true);

    /**
     * @brief 格式化HEX显示（带行号和ASCII对照）
     * @param data 字节数据
     * @param length 数据长度
     * @return 格式化的字符串
     *
     * 示例输出:
     * 0000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 0D 0A       Hello World!..
     */
    static std::string FormatHexDump(const unsigned char* data, int length);

    /**
     * @brief 判断字符是否可打印
     * @param ch 字符
     * @return 可打印返回true，否则返回false
     */
    static bool IsPrintable(unsigned char ch);

    /**
     * @brief 将字节数组从指定编码转换为UTF-8字符串
     * @param data 原始字节数组
     * @param length 数据长度
     * @param encoding 源编码类型
     * @return UTF-8编码的字符串
     */
    static std::string ConvertToUTF8(const unsigned char* data, int length, EncodingType encoding);

    /**
     * @brief 将UTF-8字符串转换为指定编码的字节数组
     * @param utf8Str UTF-8字符串
     * @param encoding 目标编码类型
     * @param outData 输出字节数组
     * @return 成功返回true，失败返回false
     */
    static bool ConvertFromUTF8(const std::string& utf8Str, EncodingType encoding, std::vector<unsigned char>& outData);

private:
    /**
     * @brief 将单个字节转换为HEX字符
     * @param byte 字节值
     * @return 两位HEX字符串
     */
    static std::string ByteToHex(unsigned char byte);

    /**
     * @brief 将HEX字符转换为数值
     * @param ch HEX字符
     * @param value 输出数值
     * @return 成功返回true，失败返回false
     */
    static bool HexCharToValue(char ch, unsigned char& value);
};

#endif // DATA_CONVERTER_H
