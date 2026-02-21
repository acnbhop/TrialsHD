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

#if REDLYNX_POSIX
#include <unistd.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using byte = std::byte;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_TOOLS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
inline void WriteBE32(std::vector<uint8>& buffer, size offset, uint32 value)
{
    buffer.push_back((value >> 24) & 0xFF);
    buffer.push_back((value >> 16) & 0xFF);
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

//
// Writes a 16-bit big-endian integer to the given buffer at the given offset.
//
inline void WriteBE16(std::vector<uint8>& buffer, size offset, uint16 value)
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
    WriteBE32(buffer, buffer.size(), be_int);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_TOOLS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
