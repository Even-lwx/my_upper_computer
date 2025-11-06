/**
 * @file RawDataParser.h
 * @brief RawData协议解析器
 * @author AI Assistant
 * @date 2025
 *
 * RawData协议格式：
 * - 原始字节流，支持多种数据类型
 * - 每个通道可以配置不同的数据类型
 * - 按照配置的类型顺序解析
 *
 * 示例配置：CH0=float, CH1=int16, CH2=uint8
 * 数据流：[4字节float][2字节int16][1字节uint8]...循环
 */

#ifndef RAWDATA_PARSER_H
#define RAWDATA_PARSER_H

#include "ProtocolParser.h"
#include <cstring>
#include <vector>

/**
 * @brief RawData协议解析器
 */
class RawDataParser : public ProtocolParser {
public:
    RawDataParser()
        : buffer_index_(0)
        , current_channel_(0)
    {
        // 默认配置：4个float通道
        SetDefaultChannels(4);
    }

    ParseResult Parse(const unsigned char* buffer, size_t length) override {
        ParseResult result;
        result.values.reserve(channel_types_.size());

        size_t consumed = 0;
        size_t i = 0;

        while (i < length) {
            if (channel_types_.empty()) {
                result.error_message = "No channel types configured";
                break;
            }

            // 当前通道需要的字节数
            size_t bytes_needed = GetDataTypeSize(channel_types_[current_channel_]);

            // 将数据拷贝到临时缓冲区
            while (buffer_index_ < bytes_needed && i < length) {
                temp_buffer_[buffer_index_++] = buffer[i++];
                consumed++;
            }

            // 如果凑齐所需字节数，解析数据
            if (buffer_index_ == bytes_needed) {
                float value = BytesToFloat(temp_buffer_, channel_types_[current_channel_]);
                result.values.push_back(value);
                buffer_index_ = 0;
                current_channel_++;

                // 如果所有通道都解析完，返回结果
                if (current_channel_ >= channel_types_.size()) {
                    current_channel_ = 0;
                    result.success = true;
                    result.bytes_consumed = consumed;
                    return result;
                }
            }
        }

        // 部分数据也返回
        if (!result.values.empty()) {
            result.success = true;
        }

        result.bytes_consumed = consumed;
        return result;
    }

    void Reset() override {
        buffer_index_ = 0;
        current_channel_ = 0;
    }

    ProtocolType GetType() const override {
        return ProtocolType::RAWDATA;
    }

    std::string GetName() const override {
        return "RawData";
    }

    size_t GetExpectedChannelCount() const override {
        return channel_types_.size();
    }

    /**
     * @brief 设置通道数据类型配置
     * @param types 数据类型数组
     */
    void SetChannelTypes(const std::vector<DataType>& types) {
        channel_types_ = types;
        Reset();
    }

    /**
     * @brief 获取通道数据类型配置
     */
    const std::vector<DataType>& GetChannelTypes() const {
        return channel_types_;
    }

    /**
     * @brief 设置默认通道（全部为float）
     */
    void SetDefaultChannels(size_t count) {
        channel_types_.clear();
        channel_types_.resize(count, DataType::FLOAT);
        Reset();
    }

private:
    std::vector<DataType> channel_types_;   // 每个通道的数据类型
    unsigned char temp_buffer_[8];          // 临时缓冲区（最大8字节）
    size_t buffer_index_;                   // 当前缓冲区索引
    size_t current_channel_;                // 当前正在解析的通道
};

#endif // RAWDATA_PARSER_H
