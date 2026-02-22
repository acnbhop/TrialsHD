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

    //
    // Wtf RedLynx...
    //
    // 1. zlib streams, RedLynx completely ditched LZMA for some tracks and compressed them using
    //    zlib deflate...
    //
    // 2. Variable headers, some lack the DEADBABE Xbox header and use a tiny 01 FF or DEADBEEF marker, meaning
    //    the 13-byte LZMA header is left entirely intact inside the file.
    //
    // 3. Raw XML payloads, files decompressed, yes, but didn't contain any OBJ5 binary structs, why? RedLynx
    //    literally just compressed raw plain-text XML directly into the .trk file instead of fucking compiling
    //    them? Why? I have no fucking clue.
    //
    // I have no idea why RedLynx did this, maybe save space? Different tool versions? I don't know, don't care, just
    // need this shit to work. But it all of a sudden changing up on me is not making me very happy right now.
    //
    // Oh, wanna know some more fuckery! They left older legacy maps, e.g., OBJ1, OBJ2, OBJ4 in the tracks! track101
    // has an LZMA error, they forgot to strip LZMA size, they left DEADBABE wrapper, and it pushed a native 8-byte
    // size directly into the encoder causing LZMA_DATA_ERROR!! Some tracks use DEADBEEF or 01 FF instead of DEADBEBE
    // which leaves 13-byte LZMA header completely intact instead of stripping the size off!!!
    //
    // Quick question... before releasing a game, don't you wanna make sure the assets are consistent in the
    // versioning at least? This seems kinda stupid having multiple versions of the same format in the codebase
    // instead of updating it all to one w/o losing or screwing up data... That's what I'd do...
    //

    std::string                 Compression;    // "LZMA", "ZLIB" or "NONE"
    std::string                 PayloadType;    // "OBJ5" or "XML"
    std::string                 RawXML;         // Holds the data if PayloadType is "XML"

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
    // Scans the file to detect compression stream and decompresses it.
    std::vector<uint8> _Decompress(const std::vector<uint8>& FileData);

    // Compresses the given data using LZMA and returns the compressed data.
    std::vector<uint8> _CompressLZMA(const std::vector<uint8>& FileData);
    // Compresses the given data using zlib and returns the compressed data.
    std::vector<uint8> _CompressZLIB(const std::vector<uint8>& FileData);

    // Parses the given decompressed track data and fills the Objects and Joints vectors. Returns true on success.
    bool _OBJ5_Parse(const std::vector<uint8>& RawData);
    // Finds a chunk with the given tag in the given buffer and returns its offset. Returns 0 if not found.
    size _FindChunk(const std::span<const uint8> Buffer, const std::string& Tag) const;
};

} // namespace redlynx::game
