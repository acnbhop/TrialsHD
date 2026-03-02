//
// gfx.hpp: Human-readable GFX data helpers.
//
// Provides structured parsing of known GFX fields for display and editing.
// The asset/gfx.hpp parser stores the raw binary blob for byte-accurate
// round-tripping. This layer extracts higher-level meaning from it.
//

#pragma once

#include "game/asset/gfx.hpp"

#include <vector>
#include <string>

namespace redlynx::game
{

//
// A transform extracted from the GFX node data.
//
struct GfxTransform
{
    f32 ScaleX, ScaleY, ScaleZ;
    f32 RotX, RotY, RotZ;
    f32 PosX, PosY, PosZ;
};

//
// Describes known fields extracted from a GFX node's data region.
//
struct GfxNodeDetail
{
    std::string  Name;
    std::string  Type;
    uint32       TypeHash;
    uint32       InstanceHash;
    std::string  ID;
    size         Offset;
    size         DataOffset;
};

//
// Structured view of a GFX file's known fields.
// Built from a Gfx object, provides human-readable access to root properties,
// node details, and identified strings.
//
class GfxDetail
{
public:
    // Root node info
    uint32                  Version;
    std::string             RootTypeName;
    uint32                  RootTypeHash;
    size                    RootDataOffset;

    // Known root property fields (offsets 4-87)
    uint32                  PropertyCode;   // uint32 at offset 4 (13 for SceneObject, 7 for Rig)

    // EFBEEFBE info
    size                    BeefOffset;
    uint32                  PostDataSize;   // uint32 immediately after EFBEEFBE

    // All identified child nodes
    std::vector<GfxNodeDetail> NodeDetails;

    // All identified strings with offsets
    std::vector<std::pair<size, std::string>> StringTable;

    // File size
    size                    FileSize;

    // Populates this GfxDetail from a parsed Gfx object.
    void Build(const Gfx& Source);

    // Prints a detailed human-readable dump of the GFX structure.
    void PrintDetail() const;
};

} // namespace redlynx::game
