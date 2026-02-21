//
// be_util.hpp: Big-endian utilities for Trials HD
// research.
//

#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>

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

} // namespace redlynx::tools::be_util
