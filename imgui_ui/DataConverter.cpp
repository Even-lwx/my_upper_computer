/**
 * @file DataConverter.cpp
 * @brief pnlbåwž°
 */

#include "DataConverter.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

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
    // ûdz<ŒyŠW&
    for (char ch : hexStr) {
        if (std::isxdigit(ch)) {
            cleanHex += ch;
        }
    }

    // HEXW&2Å{/vp¦
    if (cleanHex.length() % 2 != 0) {
        return false;
    }

    // lb
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
        // LOû0@
        oss << std::setw(4) << std::setfill('0') << std::hex << offset << ": ";

        // HEXpn
        int lineBytes = std::min(bytesPerLine, length - offset);
        for (int i = 0; i < bytesPerLine; i++) {
            if (i < lineBytes) {
                oss << ByteToHex(data[offset + i]) << " ";
            } else {
                oss << "   ";
            }
        }

        // ASCIIùg
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
    return (ch >= 32 && ch <= 126);  // ïSpASCIIW&ô
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
