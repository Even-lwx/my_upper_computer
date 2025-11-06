/**
 * @file DataTypes.h
 * @brief 数据类型定义 - 支持多种数据格式
 * @author AI Assistant
 * @date 2025
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>
#include <string>

/**
 * @brief 支持的数据类型枚举
 */
enum class DataType {
    FLOAT,      // 4字节浮点数
    INT32,      // 4字节有符号整数
    UINT32,     // 4字节无符号整数
    INT16,      // 2字节有符号整数
    UINT16,     // 2字节无符号整数
    INT8,       // 1字节有符号整数
    UINT8       // 1字节无符号整数
};

/**
 * @brief 获取数据类型的字节大小
 */
inline size_t GetDataTypeSize(DataType type) {
    switch (type) {
        case DataType::FLOAT:
        case DataType::INT32:
        case DataType::UINT32:
            return 4;
        case DataType::INT16:
        case DataType::UINT16:
            return 2;
        case DataType::INT8:
        case DataType::UINT8:
            return 1;
        default:
            return 0;
    }
}

/**
 * @brief 获取数据类型的名称
 */
inline std::string GetDataTypeName(DataType type) {
    switch (type) {
        case DataType::FLOAT:  return "float";
        case DataType::INT32:  return "int32";
        case DataType::UINT32: return "uint32";
        case DataType::INT16:  return "int16";
        case DataType::UINT16: return "uint16";
        case DataType::INT8:   return "int8";
        case DataType::UINT8:  return "uint8";
        default: return "unknown";
    }
}

/**
 * @brief 将原始字节转换为float值
 */
inline float BytesToFloat(const unsigned char* data, DataType type) {
    switch (type) {
        case DataType::FLOAT:
            return *reinterpret_cast<const float*>(data);
        case DataType::INT32:
            return static_cast<float>(*reinterpret_cast<const int32_t*>(data));
        case DataType::UINT32:
            return static_cast<float>(*reinterpret_cast<const uint32_t*>(data));
        case DataType::INT16:
            return static_cast<float>(*reinterpret_cast<const int16_t*>(data));
        case DataType::UINT16:
            return static_cast<float>(*reinterpret_cast<const uint16_t*>(data));
        case DataType::INT8:
            return static_cast<float>(*reinterpret_cast<const int8_t*>(data));
        case DataType::UINT8:
            return static_cast<float>(*data);
        default:
            return 0.0f;
    }
}

/**
 * @brief 协议类型枚举
 */
enum class ProtocolType {
    FIREWATER,      // FireWater协议（0xAA + float*N + 0x7F）
    JUSTFLOAT,      // JustFloat协议（纯float流）
    RAWDATA,        // RawData协议（原始字节）
    CUSTOM          // 自定义协议（可配置）
};

/**
 * @brief 获取协议类型名称
 */
inline std::string GetProtocolName(ProtocolType type) {
    switch (type) {
        case ProtocolType::FIREWATER: return "FireWater";
        case ProtocolType::JUSTFLOAT: return "JustFloat";
        case ProtocolType::RAWDATA:   return "RawData";
        case ProtocolType::CUSTOM:    return "Custom";
        default: return "Unknown";
    }
}

/**
 * @brief 数据点结构（时间戳 + 值）
 */
struct DataPoint {
    double timestamp;  // 时间戳（秒）
    float value;       // 数据值

    DataPoint() : timestamp(0.0), value(0.0f) {}
    DataPoint(double t, float v) : timestamp(t), value(v) {}
};

/**
 * @brief 通道配置信息
 */
struct ChannelConfig {
    bool enabled;            // 是否启用
    std::string name;        // 通道名称
    float color[4];          // 颜色（RGBA）
    DataType dataType;       // 数据类型
    float scale;             // 缩放系数
    float offset;            // 偏移量

    ChannelConfig()
        : enabled(false)
        , name("")
        , color{1.0f, 1.0f, 1.0f, 1.0f}
        , dataType(DataType::FLOAT)
        , scale(1.0f)
        , offset(0.0f)
    {}
};

#endif // DATA_TYPES_H
