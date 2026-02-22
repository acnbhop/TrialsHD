//
// texture.hpp: Definition of game texture.
//

#pragma once

// Standard headers
#include <vector>
#include <string>

namespace redlynx::game
{

//
// Texture class, contains all data and functions for loading .tex files
// and extracting to .dds files.
//
class Texture
{
public:
    uint32              Width;
    uint32              Height;
    uint32              MipCount;
    std::string         Format;
    std::vector<uint8>  Data;

    // Loads a .tex file, reads T4X header, parses texture data payload.
    bool LoadTex(const std::string& FilePath);
    // Wraps the extracted paylaod in a DirectDraw Surface (DDS) header and saves to disk.
    bool SaveDDS(const std::string& FilePath) const;
};

} // namespace redlynx::game
