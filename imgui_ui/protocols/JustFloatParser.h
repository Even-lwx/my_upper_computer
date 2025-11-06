/**
 * @file JustFloatParser.h
 * @brief JustFloat协议解析器
 * @author AI Assistant
 * @date 2025
 *
 * JustFloat协议格式：
 * - 纯float数据流（无帧头帧尾）
 * - 每4个字节解析为一个float
 * - 按顺序分配到各通道
 *
 * 示例（2通道循环）：
 * [float1] [float2] [float1] [float2] ...
 *   CH0      CH1      CH0      CH1
 */

#ifndef JUSTFLOAT_PARSER_H
#define JUSTFLOAT_PARSER_H

#include "ProtocolParser.h"
#include <cstring>

/**
 * @brief JustFloat协议解析器
 */
class JustFloatParser : public ProtocolParser {
public:
    JustFloatParser(size_t channel_count = 4)
        : channel_count_(channel_count)
        , buffer_index_(0)
    {
    }

    ParseResult Parse(const unsigned char* buffer, size_t length) override {
        ParseResult result;
        result.values.reserve(channel_count_);

        size_t consumed = 0;
        size_t i = 0;

        while (i < length) {
            // 将数据拷贝到内部缓冲区
            while (buffer_index_ < 4 && i < length) {
                temp_buffer_[buffer_index_++] = buffer[i++];
                consumed++;
            }

            // 如果凑齐4个字节，解析为float
            if (buffer_index_ == 4) {
                float value;
                std::memcpy(&value, temp_buffer_, 4);
                result.values.push_back(value);
                buffer_index_ = 0;  // 重置缓冲区

                // 如果已经读满通道数，返回结果
                if (result.values.size() >= channel_count_) {
                    result.success = true;
                    result.bytes_consumed = consumed;
                    return result;
                }
            }
        }

        // 如果有部分数据，也返回（用于多通道情况）
        if (!result.values.empty()) {
            result.success = true;
        }

        result.bytes_consumed = consumed;
        return result;
    }

    void Reset() override {
        buffer_index_ = 0;
    }

    ProtocolType GetType() const override {
        return ProtocolType::JUSTFLOAT;
    }

    std::string GetName() const override {
        return "JustFloat";
    }

    size_t GetExpectedChannelCount() const override {
        return channel_count_;
    }

    void SetExpectedChannelCount(size_t count) override {
        if (count > 0 && count <= 16) {
            channel_count_ = count;
            Reset();
        }
    }

private:
    size_t channel_count_;                  // 通道数量
    unsigned char temp_buffer_[4];          // 临时缓冲区（4字节float）
    size_t buffer_index_;                   // 当前缓冲区索引
};

#endif // JUSTFLOAT_PARSER_H
