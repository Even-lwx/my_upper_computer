/**
 * @file ProtocolParser.h
 * @brief 协议解析器基类
 * @author AI Assistant
 * @date 2025
 *
 * 定义协议解析器接口，所有具体协议解析器继承此基类
 */

#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <vector>
#include <string>
#include "../core/DataTypes.h"

/**
 * @brief 解析结果结构
 */
struct ParseResult {
    bool success;                   // 是否解析成功
    std::vector<float> values;      // 解析出的多通道数据
    size_t bytes_consumed;          // 消耗的字节数
    std::string error_message;      // 错误信息

    ParseResult()
        : success(false)
        , bytes_consumed(0)
    {}
};

/**
 * @brief 协议解析器基类（抽象类）
 */
class ProtocolParser {
public:
    virtual ~ProtocolParser() = default;

    /**
     * @brief 解析数据
     * @param buffer 输入缓冲区
     * @param length 缓冲区长度
     * @return 解析结果
     */
    virtual ParseResult Parse(const unsigned char* buffer, size_t length) = 0;

    /**
     * @brief 重置解析器状态
     */
    virtual void Reset() = 0;

    /**
     * @brief 获取协议类型
     */
    virtual ProtocolType GetType() const = 0;

    /**
     * @brief 获取协议名称
     */
    virtual std::string GetName() const = 0;

    /**
     * @brief 获取期望的通道数量（0表示动态）
     */
    virtual size_t GetExpectedChannelCount() const {
        return 0;  // 默认动态
    }

    /**
     * @brief 设置期望的通道数量（用于某些协议）
     */
    virtual void SetExpectedChannelCount(size_t count) {
        (void)count;  // 默认实现忽略
    }
};

#endif // PROTOCOL_PARSER_H
