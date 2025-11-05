#include "SerialPort_Win.h"
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <cstring>
#include <chrono>

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
    std::vector<std::string> ports;
    for (int i = 1; i <= 256; i++) {
        std::string portName = "\\\\.\\COM" + std::to_string(i);
        HANDLE hPort = CreateFileA(
            portName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );
        if (hPort != INVALID_HANDLE_VALUE) {
            ports.push_back("COM" + std::to_string(i));
            CloseHandle(hPort);
        }
    }
    if (ports.empty()) {
        for (int i = 1; i <= 10; i++) {
            ports.push_back("COM" + std::to_string(i));
        }
    }
    return ports;
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
