//
// util.hpp
//      Utility functions for Trials HD tools and research.
//

#pragma once

#include "shared/build_defines.hpp"

#include <span>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

#if REDLYNX_POSIX
#include <unistd.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using byte = std::byte;

#if 0
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_TOOLS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

inline constexpr int8 int8_max = std::numeric_limits<int8>::max();
inline constexpr int8 int8_min = std::numeric_limits<int8>::min();
inline constexpr uint8 uint8_max = std::numeric_limits<uint8>::max();
inline constexpr uint8 uint8_min = std::numeric_limits<uint8>::min();
inline constexpr int16 int16_max = std::numeric_limits<int16>::max();
inline constexpr int16 int16_min = std::numeric_limits<int16>::min();
inline constexpr uint16 uint16_max = std::numeric_limits<uint16>::max();
inline constexpr uint16 uint16_min = std::numeric_limits<uint16>::min();
inline constexpr int32 int32_max = std::numeric_limits<int32>::max();
inline constexpr int32 int32_min = std::numeric_limits<int32>::min();
inline constexpr uint32 uint32_max = std::numeric_limits<uint32>::max();
inline constexpr uint32 uint32_min = std::numeric_limits<uint32>::min();
inline constexpr int64 int64_max = std::numeric_limits<int64>::max();
inline constexpr int64 int64_min = std::numeric_limits<int64>::min();
inline constexpr uint64 uint64_max = std::numeric_limits<uint64>::max();
inline constexpr uint64 uint64_min = std::numeric_limits<uint64>::min();
inline constexpr f32 f32_max = std::numeric_limits<f32>::max();
inline constexpr f32 f32_min = std::numeric_limits<f32>::lowest();
inline constexpr f64 f64_max = std::numeric_limits<f64>::max();
inline constexpr f64 f64_min = std::numeric_limits<f64>::lowest();
inline constexpr f32 f32_minimum = std::numeric_limits<f32>::min();
inline constexpr f64 f64_minimum = std::numeric_limits<f64>::min();
inline constexpr f32 f32_epsilon = std::numeric_limits<f32>::epsilon();
inline constexpr f64 f64_epsilon = std::numeric_limits<f64>::epsilon();
inline constexpr f32 f32_infinity = std::numeric_limits<f32>::infinity();
inline constexpr f64 f64_infinity = std::numeric_limits<f64>::infinity();
inline constexpr intmax intmax_max = std::numeric_limits<intmax>::max();
inline constexpr intmax intmax_min = std::numeric_limits<intmax>::min();
inline constexpr uintmax uintmax_max = std::numeric_limits<uintmax>::max();
inline constexpr uintmax uintmax_min = std::numeric_limits<uintmax>::min();
inline constexpr size size_max = std::numeric_limits<size>::max();
inline constexpr size size_min = std::numeric_limits<size>::min();
inline constexpr ptrdiff ptrdiff_max = std::numeric_limits<ptrdiff>::max();
inline constexpr ptrdiff ptrdiff_min = std::numeric_limits<ptrdiff>::min();

//
// Converts a byte vector to a hex string.
//
inline std::string ToHex(const std::vector<uint8>& data)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int32 b : data)
    {
        ss << std::setw(2) << b;
    }

    return ss.str();
}

//
// Converts a single hex character to its byte value. Returns 0 for invalid characters.
//
inline uint8 HexCharToByte(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }

    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }

    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }

    return 0;
}

//
// Converts a hex string to a byte vector. The input string must have an even length and contain only valid hex
// characters.
//
inline std::vector<uint8> FromHex(const std::string& hex)
{
    std::vector<uint8> res;

    for (size i = 0; i < hex.length(); i += 2)
    {
        res.push_back((HexCharToByte(hex[i]) << 4) | HexCharToByte(hex[i + 1]));
    }

    return res;
}

//
// Reads a 32-bit big-endian integer from the given buffer at the given offset.
//
inline uint32 ReadBE32(std::span<const uint8> buffer, size offset)
{
    if (offset + 4 > buffer.size())
    {
        throw std::out_of_range("Offset it out of bounds for the given buffer.");
    }

    return (static_cast<uint32>(buffer[offset]) << 24)          |
           (static_cast<uint32>(buffer[offset + 1]) << 16)      |
           (static_cast<uint32>(buffer[offset + 2]) << 8)       |
           (static_cast<uint32>(buffer[offset + 3]));
}

//
// Reads a 16-bit big-endian integer from the given buffer at the given offset.
//
inline uint16 ReadBE16(std::span<const uint8> buffer, size offset)
{
    if (offset + 2 > buffer.size())
    {
        throw std::out_of_range("Offset it out of bounds for the given buffer.");
    }

    return (static_cast<uint16>(buffer[offset]) << 8)       |
           (static_cast<uint16>(buffer[offset + 1]));
}

//
// Reads a big-endian float from the given buffer at the given offset.
//
inline f32 ReadBEFloat(std::span<const uint8> buffer, size offset)
{
    uint32 be_int = ReadBE32(buffer, offset);
    f32 f;
    std::memcpy(&f, &be_int, sizeof(f));
    return f;
}

//
// Writes a 32-bit big-endian integer to the given buffer at the given offset.
//
inline void WriteBE32(std::vector<uint8>& buffer, uint32 value)
{
    buffer.push_back((value >> 24) & 0xFF);
    buffer.push_back((value >> 16) & 0xFF);
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

//
// Writes a 16-bit big-endian integer to the given buffer at the given offset.
//
inline void WriteBE16(std::vector<uint8>& buffer, uint16 value)
{
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

//
// Writes a big-endian float to the given buffer at the given offset.
//
inline void WriteBEFloat(std::vector<uint8>& buffer, f32 value)
{
    uint32 be_int;
    std::memcpy(&be_int, &value, sizeof(be_int));
    WriteBE32(buffer, be_int);
}

//
// Decodes a single UTF-16BE code unit pair from buffer at the given byte offset.
// Returns the decoded Unicode code point and advances the offset past the consumed bytes.
// Handles surrogate pairs for characters outside the BMP.
//
inline static uint32 _DecodeUTF16BE(const std::vector<uint8>& Data, size& Pos)
{
    if (Pos + 2 > Data.size()) return 0;
    uint16 Hi = (static_cast<uint16>(Data[Pos]) << 8) | Data[Pos + 1];
    Pos += 2;

    // Check for surrogate pair
    if (Hi >= 0xD800 && Hi <= 0xDBFF && Pos + 2 <= Data.size())
    {
        uint16 Lo = (static_cast<uint16>(Data[Pos]) << 8) | Data[Pos + 1];
        if (Lo >= 0xDC00 && Lo <= 0xDFFF)
        {
            Pos += 2;
            return 0x10000 + ((static_cast<uint32>(Hi - 0xD800) << 10) | (Lo - 0xDC00));
        }
    }

    return Hi;
}

//
// Encodes a Unicode code point as UTF-8 and appends to the given string.
//
inline static void _EncodeUTF8(std::string& Out, uint32 CodePoint)
{
    if (CodePoint <= 0x7F)
    {
        Out += static_cast<char>(CodePoint);
    }
    else if (CodePoint <= 0x7FF)
    {
        Out += static_cast<char>(0xC0 | (CodePoint >> 6));
        Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
    }
    else if (CodePoint <= 0xFFFF)
    {
        Out += static_cast<char>(0xE0 | (CodePoint >> 12));
        Out += static_cast<char>(0x80 | ((CodePoint >> 6) & 0x3F));
        Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
    }
    else if (CodePoint <= 0x10FFFF)
    {
        Out += static_cast<char>(0xF0 | (CodePoint >> 18));
        Out += static_cast<char>(0x80 | ((CodePoint >> 12) & 0x3F));
        Out += static_cast<char>(0x80 | ((CodePoint >> 6) & 0x3F));
        Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
    }
}

//
// Decodes a single UTF-8 code point from the string at the given byte offset.
// Returns the Unicode code point and advances the offset.
//
inline static uint32 _DecodeUTF8(const std::string& Str, size& Pos)
{
    if (Pos >= Str.size()) return 0;
    uint8 c = static_cast<uint8>(Str[Pos]);

    if (c <= 0x7F)
    {
        Pos++;
        return c;
    }
    else if ((c & 0xE0) == 0xC0 && Pos + 1 < Str.size())
    {
        uint32 cp = (c & 0x1F) << 6;
        cp |= (static_cast<uint8>(Str[Pos + 1]) & 0x3F);
        Pos += 2;
        return cp;
    }
    else if ((c & 0xF0) == 0xE0 && Pos + 2 < Str.size())
    {
        uint32 cp = (c & 0x0F) << 12;
        cp |= (static_cast<uint8>(Str[Pos + 1]) & 0x3F) << 6;
        cp |= (static_cast<uint8>(Str[Pos + 2]) & 0x3F);
        Pos += 3;
        return cp;
    }
    else if ((c & 0xF8) == 0xF0 && Pos + 3 < Str.size())
    {
        uint32 cp = (c & 0x07) << 18;
        cp |= (static_cast<uint8>(Str[Pos + 1]) & 0x3F) << 12;
        cp |= (static_cast<uint8>(Str[Pos + 2]) & 0x3F) << 6;
        cp |= (static_cast<uint8>(Str[Pos + 3]) & 0x3F);
        Pos += 4;
        return cp;
    }

    Pos++;
    return 0xFFFD; // Replacement character
}

//
// Encodes a Unicode code point as UTF-16BE and appends to the given byte vector.
//
inline static void _EncodeUTF16BE(std::vector<uint8>& Out, uint32 CodePoint)
{
    if (CodePoint <= 0xFFFF)
    {
        Out.push_back(static_cast<uint8>((CodePoint >> 8) & 0xFF));
        Out.push_back(static_cast<uint8>(CodePoint & 0xFF));
    }
    else if (CodePoint <= 0x10FFFF)
    {
        CodePoint -= 0x10000;
        uint16 Hi = 0xD800 + static_cast<uint16>((CodePoint >> 10) & 0x3FF);
        uint16 Lo = 0xDC00 + static_cast<uint16>(CodePoint & 0x3FF);
        Out.push_back(static_cast<uint8>((Hi >> 8) & 0xFF));
        Out.push_back(static_cast<uint8>(Hi & 0xFF));
        Out.push_back(static_cast<uint8>((Lo >> 8) & 0xFF));
        Out.push_back(static_cast<uint8>(Lo & 0xFF));
    }
}

//
// Converts a UTF-16BE byte sequence to a UTF-8 string.
//
inline static std::string _UTF16BEToUTF8(const std::vector<uint8>& Data, size Offset, size CharCount)
{
    std::string Result;
    size Pos = Offset;
    size End = Offset + (CharCount * 2);
    if (End > Data.size()) End = Data.size();

    while (Pos < End)
    {
        uint32 CodePoint = _DecodeUTF16BE(Data, Pos);
        _EncodeUTF8(Result, CodePoint);
    }

    return Result;
}

//
// Converts a UTF-8 string to UTF-16BE bytes and returns the char count (in UTF-16 code units).
//
inline static uint16 _UTF8ToUTF16BE(const std::string& Str, std::vector<uint8>& Out)
{
    uint16 CharCount = 0;
    size Pos = 0;

    while (Pos < Str.size())
    {
        uint32 CodePoint = _DecodeUTF8(Str, Pos);
        size Before = Out.size();
        _EncodeUTF16BE(Out, CodePoint);
        CharCount += static_cast<uint16>((Out.size() - Before) / 2);
    }

    return CharCount;
}

//
// Escapes carriage-return characters so they survive XML round-tripping.
// \r -> \\r, \\ -> \\\\ (backslash-escaped).
//
inline std::string EscapeCR(const std::string& Input)
{
    std::string Result;
    Result.reserve(Input.size());
    for (char C : Input)
    {
        if (C == '\\') Result += "\\\\";
        else if (C == '\r') Result += "\\r";
        else Result.push_back(C);
    }
    return Result;
}

//
// Reverses EscapeCR: \\r -> \r, \\\\ -> \\.
//
inline std::string UnescapeCR(const std::string& Input)
{
    std::string Result;
    Result.reserve(Input.size());
    for (size I = 0; I < Input.size(); I++)
    {
        if (Input[I] == '\\' && I + 1 < Input.size())
        {
            if (Input[I + 1] == 'r')       { Result.push_back('\r'); I++; }
            else if (Input[I + 1] == '\\') { Result.push_back('\\'); I++; }
            else Result.push_back(Input[I]);
        }
        else Result.push_back(Input[I]);
    }
    return Result;
}

#if 0
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_TOOLS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
