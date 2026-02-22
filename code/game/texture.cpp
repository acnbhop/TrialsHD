//
// texture.cpp
//

// File header
#include "texture.hpp"

// Standard headers
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace redlynx::game
{

struct DDSPixelFormat
{
    uint32          dwSize;
    uint32          dwFlags;
    uint32          dwFourCC;
    uint32          dwRGBBitCount;
    uint32          dwRBitMask;
    uint32          dwGBitMask;
    uint32          dwBBitMask;
    uint32          dwABitMask;
};

struct DDSHeader
{
    uint32 dwSize;
    uint32 dwFlags;
    uint32 dwHeight;
    uint32 dwWidth;
    uint32 dwPitchOrLinearSize;
    uint32 dwDepth;
    uint32 dwMipMapCount;
    uint32 dwReserved1[11];
    DDSPixelFormat ddspf;
    uint32 dwCaps;
    uint32 dwCaps2;
    uint32 dwCaps3;
    uint32 dwCaps4;
    uint32 dwReserved2;
};

// Loads a .tex file, reads T4X header, parses texture data payload.
bool Texture::LoadTex(const std::string& FilePath)
{
    std::ifstream File(FilePath, std::ios::binary | std::ios::ate);

    if (!File)
    {
        std::cerr << "[-] Failed to open file: " << FilePath << std::endl;
        return false;
    }

    size FileSize = static_cast<size>(File.tellg());
    File.seekg(0, std::ios::beg);

    // Header size for T4X fixes 28 bytes.
    if (FileSize < 28)
    {
        std::cerr << "[-] File too small to be a valid .tex: " << FilePath << std::endl;
        return false;
    }

    std::vector<uint8> Header(28);

    File.read(reinterpret_cast<char*>(Header.data()), 28);

    if (std::memcmp(Header.data(), "T4X\0", 4) != 0)
    {
        std::cerr << "[-] Invalid T4X header in file: " << FilePath << std::endl;
        return false;
    }

    // Defaulting to DXT5.
    //
    // Xbox T4X handles are overwhemingly DXT5 natively for maps/ncombines.
    Format = "DXT5";
    
    size DataSize = FileSize - 28;
    Data.resize(DataSize);
    File.read(reinterpret_cast<char*>(Data.data()), DataSize);

    // Calculate mathematical resolution mapping based on payload structure.
    
    uint32 BlockBytes = (Format == "DXT5") ? 16 : 8;
    uint32 TargetBlocks = static_cast<uint32>(DataSize / BlockBytes);

    Width = 0;
    Height = 0;
    MipCount = 0;

    for (uint32 w = 4; w <= 8192; w *= 2)
    {
        uint32 Blocks = 0;
        uint32 MipW = w;
        uint32 Mips = 0;

        while (MipW >= 1)
        {
            uint32 BW = std::max<uint32>(1, MipW / 4);
            Blocks += BW * BW;
            Mips++;
            if (MipW == 1)
            {
                break;
            }
            MipW /= 2;
        }

        if (Blocks <= TargetBlocks && TargetBlocks <= Blocks + 64)
        {
            Width = w;
            Height = w;
            MipCount = Mips;
            break;
        }
    }

    if (Width == 0)
    {
        std::cerr << "[-] Failed to determine texture dimensions for file: " << FilePath << std::endl;
        Width = 512;
        Height = 512;
        MipCount = 10;
    }

    std::cout << "[+] Loaded .tex: " << FilePath << " (Width: " << Width << ", Height: " << Height << ", Mips: " << MipCount << ", Format: " << Format << ")" << std::endl;
    std::cout << "[!] Note: X360 textures are tiled/swizzled (Morton curve)." << std::endl;

    return true;
}

// Wraps the extracted paylaod in a DirectDraw Surface (DDS) header and saves to disk.
bool Texture::SaveDDS(const std::string& FilePath) const
{
    std::ofstream file(FilePath, std::ios::binary);
    if (!file)
    {
        std::cerr << "[-] Failed to open output file: " << FilePath << "\n";
        return false;
    }

    uint32 magic = 0x20534444; // "DDS "
    file.write(reinterpret_cast<const char*>(&magic), 4);

    DDSHeader Header = {};
    std::memset(&Header, 0, sizeof(Header));
    Header.dwSize = 124;
    // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE
    Header.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000 | 0x80000; 
    
    if (MipCount > 1)
    {
        Header.dwFlags |= 0x20000; // DDSD_MIPMAPCOUNT
    }

    Header.dwHeight = Height;
    Header.dwWidth = Width;
    Header.dwPitchOrLinearSize = std::max<uint32>(1, (Width + 3) / 4) * std::max<uint32>(1, (Height + 3) / 4) * ((Format == "DXT5") ? 16 : 8);
    Header.dwDepth = 0;
    Header.dwMipMapCount = MipCount;

    Header.ddspf.dwSize = 32;
    Header.ddspf.dwFlags = 0x4; // DDPF_FOURCC
    
    if (Format == "DXT5")
    {
        Header.ddspf.dwFourCC = 0x35545844; // "DXT5"
    } 
    else 
    {
        Header.ddspf.dwFourCC = 0x31545844; // "DXT1"
    }

    Header.dwCaps = 0x1000; // DDSCAPS_TEXTURE
    if (MipCount > 1)
    {
        Header.dwCaps |= 0x400008; // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
    }

    file.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
    file.write(reinterpret_cast<const char*>(Data.data()), Data.size());

    return true;
}

} // namespace redlynx::game
