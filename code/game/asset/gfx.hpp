//
// gfx.hpp: Graphics scene file (.gfx) parser.
//
// GFX files are serialized scene graphs used by Trials HD's RedLynx engine. They store a tree of
// scene nodes (meshes, lights, sounds, particle emitters, collision shapes, sub-scene references,
// and skeletal rigs) together with transforms, properties, and material/controller data.
//
// File layout:
//   [4 bytes]   Version (uint32, always 1)
//   [N bytes]   Root node block (recursive depth-first serialization)
//   [1 byte]    Post-data flag (0x01)
//   [4 bytes]   Magic marker: EF BE EF BE
//   [4 bytes]   Post-data payload size (uint32)
//   [M bytes]   Post-data payload (materials, animation controllers)
//   [8 bytes]   Footer
//
// Each node in the tree consists of type-specific property data, followed by an instance hash,
// type hash, ID string, name string, post-name properties, and child nodes (recursively).
//
// The entire file is stored as raw bytes for byte-accurate round-tripping. Known structural
// elements (version, node hashes/names, EFBEEFBE marker) are parsed as metadata.
//

#pragma once

#include <vector>
#include <string>

namespace redlynx::game
{

//
// Known node type hashes found in .gfx files.
//
namespace GfxTypeHash
{
    inline constexpr uint32 SceneObject = 0x73D39136;
    inline constexpr uint32 Mesh        = 0x65793C0C;
    inline constexpr uint32 Box         = 0x9E0A6A8E;
    inline constexpr uint32 Sphere      = 0xDC21446C;
    inline constexpr uint32 SphereAlt   = 0x7B250B76;
    inline constexpr uint32 Sound       = 0x2FF3AA15;
    inline constexpr uint32 Light       = 0x0D27FA8C;
    inline constexpr uint32 FireSource  = 0xC72C550B;
    inline constexpr uint32 Serialized  = 0x03988A53;
    inline constexpr uint32 Rig         = 0x00000000;
}

//
// Returns a human-readable name for a known type hash, or "Unknown" if not recognized.
//
inline const char* GfxTypeHashToString(uint32 Hash)
{
    switch (Hash)
    {
        case GfxTypeHash::SceneObject: return "SceneObject";
        case GfxTypeHash::Mesh:        return "Mesh";
        case GfxTypeHash::Box:         return "Box";
        case GfxTypeHash::Sphere:      return "Sphere";
        case GfxTypeHash::SphereAlt:   return "Sphere";
        case GfxTypeHash::Sound:       return "Sound";
        case GfxTypeHash::Light:       return "Light";
        case GfxTypeHash::FireSource:  return "FireSource";
        case GfxTypeHash::Serialized:  return "Serialized";
        case GfxTypeHash::Rig:         return "Rig";
        default:                       return "Unknown";
    }
}

//
// Describes a single identified node within the GFX file. Nodes are located by scanning
// for their instance_hash + type_hash + ID_string + name_string pattern in the raw data.
//
struct GfxNode
{
    size        Offset;         // Byte offset in file where instance_hash starts
    uint32      InstanceHash;   // Unique per node instance (0 for root)
    uint32      TypeHash;       // Identifies the node class (see GfxTypeHash)
    std::string ID;             // Numeric or short identifier string
    std::string Name;           // Human-readable node name
    size        DataOffset;     // Byte offset where post-name data starts
};

//
// GFX file class. Contains all data and functions for reading, saving, exporting and importing
// Graphics scene files. The raw file bytes are preserved exactly for byte-accurate round-tripping.
//
class Gfx
{
public:
    //
    // File version (always 1).
    //

    uint32                      Version;

    //
    // Complete raw file data. The entire binary content of the .gfx file is stored here
    // and written back as-is on save. This guarantees byte-perfect round-tripping.
    //

    std::vector<uint8>          Data;

    //
    // Root node metadata (parsed from the header area).
    //

    uint32                      RootTypeHash;       // Type hash at offset 92 (e.g. 0x73D39136 for SceneObject)
    std::string                 RootTypeName;       // Type name string (e.g. "SceneObject")
    size                        RootDataOffset;     // Byte offset where root's post-name data starts

    //
    // Identified child nodes (parsed by scanning the data for hash+string patterns).
    // These are metadata-only. Modifying them does NOT modify the underlying Data bytes.
    //

    std::vector<GfxNode>        Nodes;

    //
    // Position of the EFBEEFBE magic marker in the data, or 0 if not found.
    //

    size                        BeefOffset;

    //
    // All length-prefixed ASCII strings found in the file, with their offsets.
    // Each pair is (offset_of_length_field, string_value).
    //

    std::vector<std::pair<size, std::string>> Strings;

    // Loads a .gfx file from disk.
    bool Load(const std::string& FilePath);

    // Saves the GFX data back to a binary .gfx file (byte-for-byte from Data).
    bool Save(const std::string& FilePath);

    // Exports the GFX data to a human-readable XML format for inspection and editing.
    bool ExportXML(const std::string& FilePath) const;

    // Imports the GFX data from an XML file previously exported by ExportXML.
    bool ImportXML(const std::string& FilePath);

    // Prints a summary of the GFX file to the console.
    void PrintSummary() const;

private:
    // Scans Data for length-prefixed ASCII strings and identifies consecutive ID+name pairs
    // as nodes. Populates Nodes and Strings.
    void _IdentifyNodes();

    // Finds the EFBEEFBE magic marker in Data and sets BeefOffset.
    void _FindBeefMarker();
};

} // namespace redlynx::game
