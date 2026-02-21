//
// be_util.hpp: Big-endian utilities for Trials HD
// research.
//

#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>
#include <cstring>

namespace redlynx::tools::be_util
{

inline uint32_t read_be32(std::span<const uint8_t> buffer, size_t offset) 
{
    if (offset + 4 > buffer.size()) 
    {
        throw std::out_of_range("Offset is out of bounds for the given buffer.");
    }

    return (static_cast<uint32_t>(buffer[offset]) << 24)      |
           (static_cast<uint32_t>(buffer[offset + 1]) << 16)  |
           (static_cast<uint32_t>(buffer[offset + 2]) << 8)   |
           (static_cast<uint32_t>(buffer[offset + 3]));
}

inline uint16_t read_be16(std::span<const uint8_t> buffer, size_t offset)
{
    if (offset + 2 > buffer.size()) 
    {
        throw std::out_of_range("Offset is out of bounds for the given buffer.");
    }

    return (static_cast<uint16_t>(buffer[offset]) << 8) |
           (static_cast<uint16_t>(buffer[offset + 1]));
}

inline float read_be_float(std::span<const uint8_t> buffer, size_t offset)
{
    uint32_t be_int = read_be32(buffer, offset);
    float f;
    std::memcpy(&f, &be_int, sizeof(f));
    return f;
}

} // namespace redlynx::tools::be_util
