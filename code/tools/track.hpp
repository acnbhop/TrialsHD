#pragma once

#include "be_util.hpp"

#include <vector>
#include <string>
#include <stdexcept>
#include <span>
#include <cstring>

namespace redlynx::game
{

struct TrackObject
{
    uint8_t     TypeID;
    uint16_t    Variant;
    float       X, Y, Z;
    uint8_t     RotX, RotY, RotZ;
    uint8_t     ScaleOrExtra;
};

struct TrackJoint
{
    uint16_t ObjA;
    uint16_t ObjB;
    uint16_t Param1;
    uint16_t Param2;
};

class Track
{
public:
    std::vector<TrackObject> Objects;
    std::vector<TrackJoint> Joints;

    // Loads a .trk file, decompresses it and parses it's data.
    bool Load(const std::string& FilePath);

    // Prints a sample of extracted data.
    void PrintSummary() const;
private:
    std::vector<uint8_t> _DecompressLZMA(const std::vector<uint8_t>& FileData);
    bool _OBJ5Parse(const std::vector<uint8_t>& RawData);
    size_t FindChunk(std::span<const uint8_t> Buffer, const std::string& Tag) const;
};

} // namespace redlynx::game
