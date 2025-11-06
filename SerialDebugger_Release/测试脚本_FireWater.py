# -*- coding: utf-8 -*-
"""
串口调试助手 - FireWater协议测试脚本
用于快速测试可视化功能

使用方法：
1. 确保安装了pyserial: pip install pyserial
2. 修改COM_PORT为你的串口号
3. 运行此脚本
4. 在串口调试助手中选择FireWater协议
"""

import serial
import struct
import time
import math

# ===== 配置参数 =====
COM_PORT = 'COM3'       # 修改为你的串口号
BAUDRATE = 115200       # 波特率
SEND_RATE = 100         # 发送频率(Hz)

# ===== 初始化串口 =====
try:
    ser = serial.Serial(COM_PORT, BAUDRATE, timeout=1)
    print(f"✓ 成功打开串口: {COM_PORT}")
    print(f"✓ 波特率: {BAUDRATE}")
    print(f"✓ 发送频率: {SEND_RATE}Hz")
    print("\n开始发送测试数据...(按Ctrl+C停止)\n")
except Exception as e:
    print(f"✗ 打开串口失败: {e}")
    print("\n请检查：")
    print("  1. 串口号是否正确")
    print("  2. 串口是否被其他程序占用")
    print("  3. 是否安装了pyserial (pip install pyserial)")
    exit(1)

# ===== 发送数据 =====
packet_count = 0
start_time = time.time()

try:
    while True:
        t = time.time() - start_time

        # 生成4个通道的测试波形
        ch1 = math.sin(t * 2 * math.pi * 0.5)       # 0.5Hz正弦波
        ch2 = math.cos(t * 2 * math.pi * 1.0)       # 1Hz余弦波
        ch3 = math.sin(t * 2 * math.pi * 2.0) * 2   # 2Hz正弦波（幅度2）
        ch4 = t % 5.0                                # 0-5线性增长

        # 打包FireWater协议数据包
        # 格式: 0xAA + float*4 + 0x7F
        packet = struct.pack('<Bffff', 0xAA, ch1, ch2, ch3, ch4)
        packet += bytes([0x7F])

        # 发送数据
        ser.write(packet)
        packet_count += 1

        # 每100个包显示一次进度
        if packet_count % 100 == 0:
            print(f"已发送 {packet_count} 个数据包 | "
                  f"CH1={ch1:+.2f} CH2={ch2:+.2f} CH3={ch3:+.2f} CH4={ch4:.2f}")

        # 控制发送速率
        time.sleep(1.0 / SEND_RATE)

except KeyboardInterrupt:
    print(f"\n\n✓ 已停止发送")
    print(f"✓ 总共发送: {packet_count} 个数据包")
    print(f"✓ 运行时长: {time.time() - start_time:.1f} 秒")
except Exception as e:
    print(f"\n✗ 发送错误: {e}")
finally:
    ser.close()
    print("✓ 串口已关闭")
