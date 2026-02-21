//
// trackdef.hpp: Definition of game track.
//

#pragma once

#include "shared/build_defines.hpp"

#include <vector>
#include <string>
#include <span>

namespace redlynx::game
{

struct TrackObject
{
    uint8       TypeID;
    uint16      Variant;
    f32         X, Y, Z;
    uint8       RotX, RotY, RotZ;
    uint8       ScaleOrExtra;
};

struct TrackJoint
{
    uint16      ObjA;
    uint16      ObjB;
    uint32      ParamA;
    uint32      ParamB;
};

//
// Track class, contains all data and functions for saving, loading and manipulation.
//
class Track
{
public:
    // Objects that are in the track.
    std::vector<TrackObject>    Objects;
    // Joints that connect objects together.
    std::vector<TrackJoint>     Joints;

    //
    // Stored unknown chunks so we can ensure 1:1.
    //

    std::vector<uint8>          OriginalHeader;
    std::vector<uint8>          PreGlueData;
    std::vector<uint8>          PostGlueData;

    // Loads a .trk file, decompresses it and parses the track data.
    bool Load(const std::string& FilePath);

    // Saves the objects back into a playable .trk file.
    bool Save(const std::string& FilePath);

    // Exports the track data to a human readable XML format. This is used for editing tracks.
    bool ExportXML(const std::string& FilePath) const;
    // Imports the track data from a human readable XML format. This is used for editing tracks.
    bool ImportXML(const std::string& FilePath);

    // Prints an entire summary of the track to the console.
    void PrintSummary() const;
private:
    // Decompresses the given LZMA compressed data and returns the decompressed data.
    std::vector<uint8> _DecompressLZMA(const std::vector<uint8>& FileData);
    // Compresses the given data using LZMA and returns the compressed data.
    std::vector<uint8> _CompressLZMA(const std::vector<uint8>& FileData);
    // Parses the given decompressed track data and fills the Objects and Joints vectors. Returns true on success.
    bool _OBJ5_Parse(const std::vector<uint8>& RawData);
    // Finds a chunk with the given tag in the given buffer and returns its offset. Returns 0 if not found.
    size _FindChunk(const std::span<const uint8> Buffer, const std::string& Tag) const;
};

} // namespace redlynx::game
