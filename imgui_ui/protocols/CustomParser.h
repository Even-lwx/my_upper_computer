/**
 * @file CustomParser.h
 * @brief 自定义协议解析器
 * @author AI Assistant
 * @date 2025
 *
 * Custom协议特性：
 * - 用户可配置帧头、帧尾
 * - 用户可配置每个通道的数据类型
 * - 支持校验和验证（可选）
 *
 * 配置示例：
 * - 帧头：0x55 0xAA
 * - 通道：CH0=float, CH1=int16, CH2=uint8
 * - 帧尾：0x0D 0x0A
 */

#ifndef CUSTOM_PARSER_H
#define CUSTOM_PARSER_H

#include "ProtocolParser.h"
#include <cstring>
#include <vector>

/**
 * @brief 自定义协议配置
 */
struct CustomProtocolConfig {
    std::vector<unsigned char> frame_header;  // 帧头（可多字节）
    std::vector<unsigned char> frame_tail;    // 帧尾（可多字节）
    std::vector<DataType> channel_types;      // 通道数据类型
    bool use_checksum;                        // 是否使用校验和
    bool big_endian;                          // 是否大端序（默认小端）

    CustomProtocolConfig()
        : use_checksum(false)
        , big_endian(false)
    {
        // 默认配置：类似FireWater
        frame_header = {0xAA};
        frame_tail = {0x7F};
        channel_types = {DataType::FLOAT, DataType::FLOAT, DataType::FLOAT, DataType::FLOAT};
    }
};

/**
 * @brief 自定义协议解析器
 */
class CustomParser : public ProtocolParser {
public:
    CustomParser() : CustomParser(CustomProtocolConfig()) {}

    CustomParser(const CustomProtocolConfig& config)
        : config_(config)
        , state_(State::SEARCH_HEADER)
        , header_index_(0)
        , data_index_(0)
        , tail_index_(0)
    {
        CalculateFrameSize();
    }

    ParseResult Parse(const unsigned char* buffer, size_t length) override {
        ParseResult result;
        result.values.reserve(config_.channel_types.size());

        size_t consumed = 0;

        for (size_t i = 0; i < length; i++) {
            unsigned char byte = buffer[i];
            consumed++;

            switch (state_) {
                case State::SEARCH_HEADER:
                    if (config_.frame_header.empty() ||
                        byte == config_.frame_header[header_index_]) {
                        header_index_++;
                        if (header_index_ >= config_.frame_header.size()) {
                            // 帧头匹配成功
                            state_ = State::READ_DATA;
                            data_index_ = 0;
                            header_index_ = 0;
                        }
                    } else {
                        header_index_ = 0;
                        // 重新匹配帧头首字节
                        if (!config_.frame_header.empty() && byte == config_.frame_header[0]) {
                            header_index_ = 1;
                        }
                    }
                    break;

                case State::READ_DATA:
                    data_buffer_[data_index_++] = byte;

                    if (data_index_ >= total_data_bytes_) {
                        // 数据读取完成
                        if (config_.frame_tail.empty()) {
                            // 无帧尾，直接解析
                            if (ParseDataBuffer(result)) {
                                result.success = true;
                                result.bytes_consumed = consumed;
                                state_ = State::SEARCH_HEADER;
                                return result;
                            }
                        } else {
                            // 验证帧尾
                            state_ = State::VERIFY_TAIL;
                            tail_index_ = 0;
                        }
                    }
                    break;

                case State::VERIFY_TAIL:
                    if (byte == config_.frame_tail[tail_index_]) {
                        tail_index_++;
                        if (tail_index_ >= config_.frame_tail.size()) {
                            // 帧尾匹配成功，解析数据
                            if (ParseDataBuffer(result)) {
                                result.success = true;
                                result.bytes_consumed = consumed;
                                state_ = State::SEARCH_HEADER;
                                return result;
                            }
                        }
                    } else {
                        // 帧尾错误
                        result.error_message = "Frame tail mismatch";
                        state_ = State::SEARCH_HEADER;
                        header_index_ = 0;
                    }
                    break;
            }
        }

        result.bytes_consumed = consumed;
        return result;
    }

    void Reset() override {
        state_ = State::SEARCH_HEADER;
        header_index_ = 0;
        data_index_ = 0;
        tail_index_ = 0;
    }

    ProtocolType GetType() const override {
        return ProtocolType::CUSTOM;
    }

    std::string GetName() const override {
        return "Custom";
    }

    size_t GetExpectedChannelCount() const override {
        return config_.channel_types.size();
    }

    /**
     * @brief 设置自定义协议配置
     */
    void SetConfig(const CustomProtocolConfig& config) {
        config_ = config;
        CalculateFrameSize();
        Reset();
    }

    /**
     * @brief 获取当前配置
     */
    const CustomProtocolConfig& GetConfig() const {
        return config_;
    }

private:
    enum class State {
        SEARCH_HEADER,
        READ_DATA,
        VERIFY_TAIL
    };

    /**
     * @brief 计算数据帧大小
     */
    void CalculateFrameSize() {
        total_data_bytes_ = 0;
        for (const auto& type : config_.channel_types) {
            total_data_bytes_ += GetDataTypeSize(type);
        }
        data_buffer_.resize(total_data_bytes_);
    }

    /**
     * @brief 解析数据缓冲区
     */
    bool ParseDataBuffer(ParseResult& result) {
        size_t offset = 0;
        for (const auto& type : config_.channel_types) {
            size_t size = GetDataTypeSize(type);
            if (offset + size > data_buffer_.size()) {
                return false;
            }

            // 处理字节序
            unsigned char temp[8];
            if (config_.big_endian && size > 1) {
                // 大端转小端
                for (size_t i = 0; i < size; i++) {
                    temp[i] = data_buffer_[offset + size - 1 - i];
                }
            } else {
                std::memcpy(temp, &data_buffer_[offset], size);
            }

            float value = BytesToFloat(temp, type);
            result.values.push_back(value);
            offset += size;
        }

        return true;
    }

    CustomProtocolConfig config_;
    State state_;
    size_t header_index_;
    size_t data_index_;
    size_t tail_index_;
    size_t total_data_bytes_;
    std::vector<unsigned char> data_buffer_;
};

#endif // CUSTOM_PARSER_H
