//
// gfx.cpp: Human-readable GFX data helpers.
//

// File header
#include "gfx.hpp"

// Standard headers
#include <iostream>
#include <iomanip>
#include <cstring>

namespace redlynx::game
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32 _ReadLE32Detail(const std::vector<uint8>& Data, size Offset)
{
    if (Offset + 4 > Data.size()) return 0;
    return static_cast<uint32>(Data[Offset])
         | (static_cast<uint32>(Data[Offset + 1]) << 8)
         | (static_cast<uint32>(Data[Offset + 2]) << 16)
         | (static_cast<uint32>(Data[Offset + 3]) << 24);
}

static f32 _ReadLEFloatDetail(const std::vector<uint8>& Data, size Offset)
{
    uint32 Bits = _ReadLE32Detail(Data, Offset);
    f32 Result;
    std::memcpy(&Result, &Bits, sizeof(f32));
    return Result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GfxDetail::Build(const Gfx& Source)
{
    Version = Source.Version;
    RootTypeName = Source.RootTypeName;
    RootTypeHash = Source.RootTypeHash;
    RootDataOffset = Source.RootDataOffset;
    BeefOffset = Source.BeefOffset;
    FileSize = Source.Data.size();

    // Property code at offset 4
    PropertyCode = _ReadLE32Detail(Source.Data, 4);

    // Post-data size (uint32 immediately after EFBEEFBE)
    PostDataSize = 0;
    if (BeefOffset > 0 && BeefOffset + 8 <= Source.Data.size())
    {
        PostDataSize = _ReadLE32Detail(Source.Data, BeefOffset + 4);
    }

    // Build node details
    NodeDetails.clear();
    for (const GfxNode& Node : Source.Nodes)
    {
        GfxNodeDetail Detail;
        Detail.Name         = Node.Name;
        Detail.Type         = GfxTypeHashToString(Node.TypeHash);
        Detail.TypeHash     = Node.TypeHash;
        Detail.InstanceHash = Node.InstanceHash;
        Detail.ID           = Node.ID;
        Detail.Offset       = Node.Offset;
        Detail.DataOffset   = Node.DataOffset;
        NodeDetails.push_back(Detail);
    }

    // Copy string table
    StringTable = Source.Strings;
}

void GfxDetail::PrintDetail() const
{
    std::cout << "=== GFX Detail View ===\n\n";

    std::cout << "File Size:      " << FileSize << " bytes\n";
    std::cout << "Version:        " << Version << "\n";
    std::cout << "Property Code:  " << PropertyCode << "\n";
    std::cout << "Root Type:      " << (RootTypeName.empty() ? "(none)" : RootTypeName);
    std::cout << " (0x" << std::hex << std::setw(8) << std::setfill('0') << RootTypeHash << std::dec << ")";
    std::cout << "\n";
    std::cout << "Root Data @:    " << RootDataOffset << "\n";
    std::cout << "\n";

    // EFBEEFBE section
    if (BeefOffset > 0)
    {
        std::cout << "--- Post-Data Section ---\n";
        std::cout << "  EFBEEFBE @:     " << BeefOffset << "\n";
        std::cout << "  Payload Size:   " << PostDataSize << " bytes\n";
        std::cout << "  Scene Region:   0 .. " << (BeefOffset - 1) << " (" << BeefOffset << " bytes)\n";
        size PostTotal = FileSize - BeefOffset;
        std::cout << "  Post Region:    " << BeefOffset << " .. " << (FileSize - 1) << " (" << PostTotal << " bytes)\n";
        if (FileSize > 8)
        {
            std::cout << "  Footer:         " << (FileSize - 8) << " .. " << (FileSize - 1) << " (8 bytes)\n";
        }
        std::cout << "\n";
    }

    // Nodes
    std::cout << "--- Child Nodes (" << NodeDetails.size() << ") ---\n";
    for (size i = 0; i < NodeDetails.size(); i++)
    {
        const GfxNodeDetail& D = NodeDetails[i];
        std::cout << "  [" << std::setw(2) << std::setfill(' ') << i << "] "
                  << std::setw(18) << std::setfill(' ') << std::left << D.Name << std::right
                  << " | " << std::setw(12) << std::setfill(' ') << std::left << D.Type << std::right
                  << " | THash: 0x" << std::hex << std::setw(8) << std::setfill('0') << D.TypeHash << std::dec
                  << " | IHash: 0x" << std::hex << std::setw(8) << std::setfill('0') << D.InstanceHash << std::dec
                  << " | ID: " << D.ID
                  << " | @" << D.Offset
                  << " data@" << D.DataOffset << "\n";
    }
    std::cout << "\n";

    // String table
    std::cout << "--- String Table (" << StringTable.size() << ") ---\n";
    for (size i = 0; i < StringTable.size(); i++)
    {
        std::string Display = StringTable[i].second;
        if (Display.size() > 70)
        {
            Display = Display.substr(0, 67) + "...";
        }
        std::cout << "  @" << std::setw(6) << std::setfill(' ') << StringTable[i].first
                  << ": [" << std::setw(3) << std::setfill(' ') << StringTable[i].second.size()
                  << "] " << Display << "\n";
    }
}

} // namespace redlynx::game
