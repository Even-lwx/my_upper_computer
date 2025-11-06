#include "SerialPort_Win.h"
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <initguid.h>

// 链接SetupAPI库
#pragma comment(lib, "setupapi.lib")

// 定义串口设备类GUID
DEFINE_GUID(GUID_DEVCLASS_PORTS, 0x4D36E978, 0xE325, 0x11CE, 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18);

/**
 * @brief 将Windows宽字符串转换为UTF-8字符串
 * @param wstr 宽字符串指针
 * @return UTF-8编码的字符串
 */
static std::string WideToUtf8(const wchar_t* wstr) {
    if (!wstr || wstr[0] == L'\0') return "";

    // 计算所需缓冲区大小
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (size <= 0) return "";

    // 转换为UTF-8
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], size, NULL, NULL);
    return result;
}

SerialPort_Win::SerialPort_Win()
    : hComm_(INVALID_HANDLE_VALUE)
    , isOpen_(false)
    , isReceiving_(false)
    , receiveCallback_(nullptr)
{
}

SerialPort_Win::~SerialPort_Win() {
    Close();
}

std::vector<std::string> SerialPort_Win::EnumeratePorts() {
    // 使用详细枚举，但只返回端口名称（向后兼容）
    auto detailed = EnumeratePortsDetailed();
    std::vector<std::string> ports;
    for (const auto& info : detailed) {
        ports.push_back(info.portName);
    }

    // 如果没找到任何端口，返回默认列表
    if (ports.empty()) {
        for (int i = 1; i <= 10; i++) {
            ports.push_back("COM" + std::to_string(i));
        }
    }

    return ports;
}

/**
 * @brief 使用SetupAPI枚举串口设备（详细版）
 */
std::vector<SerialPortInfo> SerialPort_Win::EnumeratePortsDetailed() {
    std::vector<SerialPortInfo> ports;

    // 获取所有Ports设备类的设备信息集
    HDEVINFO hDevInfo = SetupDiGetClassDevsW(
        &GUID_DEVCLASS_PORTS,  // Ports设备类
        NULL,
        NULL,
        DIGCF_PRESENT          // 仅获取当前存在的设备
    );

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return ports;
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    // 枚举设备信息集中的每个设备
    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
        SerialPortInfo portInfo;
        wchar_t buffer[256] = {0};
        DWORD dataType = 0;
        DWORD bufferSize = 0;

        // 获取友好名称（包含COM端口号）
        if (SetupDiGetDeviceRegistryPropertyW(
                hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME,
                &dataType, (PBYTE)buffer, sizeof(buffer), &bufferSize)) {
            portInfo.friendlyName = WideToUtf8(buffer);

            // 从友好名称提取COM端口号
            // 示例："USB Serial Port (COM3)" -> "COM3"
            const wchar_t* start = wcsstr(buffer, L"(COM");
            if (start) {
                start++; // 跳过 '('
                const wchar_t* end = wcschr(start, L')');
                if (end && (end - start) < 20) {
                    portInfo.portName = WideToUtf8(std::wstring(start, end - start).c_str());
                }
            }
        }

        // 跳过非COM设备
        if (portInfo.portName.empty() || portInfo.portName.find("COM") != 0) {
            continue;
        }

        // 获取设备描述
        memset(buffer, 0, sizeof(buffer));
        if (SetupDiGetDeviceRegistryPropertyW(
                hDevInfo, &devInfoData, SPDRP_DEVICEDESC,
                &dataType, (PBYTE)buffer, sizeof(buffer), &bufferSize)) {
            portInfo.description = WideToUtf8(buffer);
        }

        // 获取制造商
        memset(buffer, 0, sizeof(buffer));
        if (SetupDiGetDeviceRegistryPropertyW(
                hDevInfo, &devInfoData, SPDRP_MFG,
                &dataType, (PBYTE)buffer, sizeof(buffer), &bufferSize)) {
            portInfo.manufacturer = WideToUtf8(buffer);
        }

        // 获取硬件ID
        memset(buffer, 0, sizeof(buffer));
        if (SetupDiGetDeviceRegistryPropertyW(
                hDevInfo, &devInfoData, SPDRP_HARDWAREID,
                &dataType, (PBYTE)buffer, sizeof(buffer), &bufferSize)) {
            portInfo.hardwareId = WideToUtf8(buffer);
        }

        ports.push_back(portInfo);
    }

    // 清理资源
    SetupDiDestroyDeviceInfoList(hDevInfo);

    // 按端口号排序（COM3 < COM10）
    std::sort(ports.begin(), ports.end(),
        [](const SerialPortInfo& a, const SerialPortInfo& b) {
            // 提取数字部分："COM3" -> 3
            int numA = atoi(a.portName.c_str() + 3);
            int numB = atoi(b.portName.c_str() + 3);
            return numA < numB;
        }
    );

    return ports;
}

/**
 * @brief 异步枚举串口设备
 */
std::future<std::vector<SerialPortInfo>> SerialPort_Win::EnumeratePortsAsync() {
    return std::async(std::launch::async, []() {
        return EnumeratePortsDetailed();
    });
}

bool SerialPort_Win::Open(const SerialConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (isOpen_) {
        lastError_ = "Port already open";
        return false;
    }
    std::string fullPortName = "\\\\.\\" + config.portName;
    hComm_ = CreateFileA(
        fullPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );
    if (hComm_ == INVALID_HANDLE_VALUE) {
        DWORD error = ::GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            lastError_ = "Port not found: " + config.portName;
        } else if (error == ERROR_ACCESS_DENIED) {
            lastError_ = "Port access denied: " + config.portName;
        } else {
            lastError_ = "Failed to open port, error: " + std::to_string(error);
        }
        return false;
    }
    if (!ConfigurePort(config)) {
        CloseHandle(hComm_);
        hComm_ = INVALID_HANDLE_VALUE;
        return false;
    }
    SetupComm(hComm_, 4096, 4096);
    PurgeComm(hComm_, PURGE_RXCLEAR | PURGE_TXCLEAR);
    currentConfig_ = config;
    isOpen_ = true;
    isReceiving_ = true;
    receiveThread_ = std::thread(&SerialPort_Win::ReceiveThread, this);
    return true;
}

void SerialPort_Win::Close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!isOpen_) {
            return;
        }
        isOpen_ = false;
        isReceiving_ = false;
    }
    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }
    if (hComm_ != INVALID_HANDLE_VALUE) {
        CloseHandle(hComm_);
        hComm_ = INVALID_HANDLE_VALUE;
    }
}

bool SerialPort_Win::IsOpen() const {
    return isOpen_;
}

int SerialPort_Win::Write(const unsigned char* data, int length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isOpen_ || hComm_ == INVALID_HANDLE_VALUE) {
        lastError_ = "Port not open";
        return -1;
    }
    DWORD bytesWritten = 0;
    OVERLAPPED osWrite = {0};
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL) {
        lastError_ = "Create event failed";
        return -1;
    }
    BOOL writeResult = WriteFile(hComm_, data, length, &bytesWritten, &osWrite);
    if (!writeResult) {
        if (::GetLastError() == ERROR_IO_PENDING) {
            if (WaitForSingleObject(osWrite.hEvent, 1000) == WAIT_OBJECT_0) {
                GetOverlappedResult(hComm_, &osWrite, &bytesWritten, FALSE);
            } else {
                lastError_ = "Write timeout";
                CloseHandle(osWrite.hEvent);
                return -1;
            }
        } else {
            lastError_ = "Write failed";
            CloseHandle(osWrite.hEvent);
            return -1;
        }
    }
    CloseHandle(osWrite.hEvent);
    return static_cast<int>(bytesWritten);
}

int SerialPort_Win::Write(const std::string& str) {
    return Write(reinterpret_cast<const unsigned char*>(str.c_str()), str.length());
}

void SerialPort_Win::SetReceiveCallback(std::function<void(const unsigned char*, int)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    receiveCallback_ = callback;
}

std::string SerialPort_Win::GetLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void SerialPort_Win::ClearReceiveBuffer() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (isOpen_ && hComm_ != INVALID_HANDLE_VALUE) {
        PurgeComm(hComm_, PURGE_RXCLEAR);
    }
}

void SerialPort_Win::ClearTransmitBuffer() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (isOpen_ && hComm_ != INVALID_HANDLE_VALUE) {
        PurgeComm(hComm_, PURGE_TXCLEAR);
    }
}

bool SerialPort_Win::ConfigurePort(const SerialConfig& config) {
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hComm_, &dcb)) {
        lastError_ = "Get comm state failed";
        return false;
    }
    dcb.BaudRate = config.baudRate;
    dcb.ByteSize = config.dataBits;
    switch (config.stopBits) {
        case 1: dcb.StopBits = ONESTOPBIT; break;
        case 2: dcb.StopBits = TWOSTOPBITS; break;
        default: dcb.StopBits = ONESTOPBIT; break;
    }
    switch (config.parity) {
        case 0: dcb.Parity = NOPARITY; break;
        case 1: dcb.Parity = ODDPARITY; break;
        case 2: dcb.Parity = EVENPARITY; break;
        default: dcb.Parity = NOPARITY; break;
    }
    dcb.fBinary = TRUE;
    dcb.fParity = (config.parity != 0);
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fAbortOnError = FALSE;
    if (!SetCommState(hComm_, &dcb)) {
        lastError_ = "Set comm state failed";
        return false;
    }
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    if (!SetCommTimeouts(hComm_, &timeouts)) {
        lastError_ = "Set timeouts failed";
        return false;
    }
    return true;
}

void SerialPort_Win::ReceiveThread() {
    const int BUFFER_SIZE = 4096;
    unsigned char buffer[BUFFER_SIZE];
    OVERLAPPED osReader = {0};
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osReader.hEvent == NULL) {
        return;
    }
    while (isReceiving_) {
        DWORD bytesRead = 0;
        BOOL readResult = ReadFile(hComm_, buffer, BUFFER_SIZE, &bytesRead, &osReader);
        if (readResult) {
            if (bytesRead > 0 && receiveCallback_) {
                receiveCallback_(buffer, bytesRead);
            }
        } else {
            DWORD error = ::GetLastError();
            if (error == ERROR_IO_PENDING) {
                DWORD waitResult = WaitForSingleObject(osReader.hEvent, 100);
                if (waitResult == WAIT_OBJECT_0) {
                    if (GetOverlappedResult(hComm_, &osReader, &bytesRead, FALSE)) {
                        if (bytesRead > 0 && receiveCallback_) {
                            receiveCallback_(buffer, bytesRead);
                        }
                    }
                }
            } else {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    CloseHandle(osReader.hEvent);
}
