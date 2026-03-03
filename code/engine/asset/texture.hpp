//
// texture.hpp: Trials HD texture file (.tex) parser.
//
// TEX files are the texture container format used by Trials HD's RedLynx engine on Xbox 360.
// Each file stores a single texture surface with a full mipmap chain and optional sprite-sheet
// frame metadata. Pixel data is DXT-compressed (or uncompressed ARGB8), stored in Xbox 360 GPU
// tiled layout with big-endian 16-bit word order.
//
// File layout (T4X):
//   [3 bytes]   Magic ("T4X" or "T3X")
//   [2 bytes]   Width (LE uint16)
//   [2 bytes]   Height (LE uint16)
//   [1 byte]    Pixel format (0x02=ARGB8, 0x0C=DXT1, 0x0E=DXT5)
//   [1 byte]    Mipmap level count
//   [1 byte]    Frame count (1=static, >1=sprite sheet)
//   [N×18]      Frame descriptors
//   [1-2 bytes] Trailing bytes
//   [M bytes]   Pixel data (tiled, full mip chain)
//
// Header size: 10 + FrameCount*18 + (T4X: 2, T3X: 1) bytes
//
// The pixel data is in Xbox 360 tiled (swizzled) memory layout and uses big-endian byte order
// for 16-bit values. Conversion to/from PC DDS requires untiling and byte-swapping.
//

#pragma once

#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Pixel format identifiers used in .tex files. These correspond to Xbox 360 GPUTEXTUREFORMAT values.
//
namespace TexFormat
{
	inline constexpr uint8 ARGB8 = 0x02;	// A8R8G8B8 uncompressed (32bpp, 4 bytes/pixel)
	inline constexpr uint8 DXT1  = 0x0C;	// DXT1 / BC1 compressed (4bpp, 8 bytes/4×4 block)
	inline constexpr uint8 DXT5  = 0x0E;	// DXT5 / BC3 compressed (8bpp, 16 bytes/4×4 block)
}

//
// Returns a human-readable name for a pixel format byte value.
//
inline const char* TexFormatToString(uint8 Format)
{
	switch (Format)
	{
		case TexFormat::ARGB8: return "A8R8G8B8";
		case TexFormat::DXT1:  return "DXT1";
		case TexFormat::DXT5:  return "DXT5";
		default:               return "Unknown";
	}
}

//
// Returns the number of bytes per block (or per pixel for uncompressed formats).
// DXT1: 8 bytes per 4×4 block. DXT5: 16 bytes per 4×4 block. ARGB8: 4 bytes per pixel.
//
inline uint32 TexFormatBytesPerBlock(uint8 Format)
{
	switch (Format)
	{
		case TexFormat::ARGB8: return 4;
		case TexFormat::DXT1:  return 8;
		case TexFormat::DXT5:  return 16;
		default:               return 0;
	}
}

//
// Returns true if the format is block-compressed (DXT).
//
inline bool TexFormatIsCompressed(uint8 Format)
{
	return Format == TexFormat::DXT1 || Format == TexFormat::DXT5;
}

//
// Describes a single frame within an animated sprite sheet (or the sole frame for static textures).
// Each frame occupies a rectangular region within the texture atlas.
//
struct TexFrame
{
	uint16 XOffset;		// X pixel offset of this frame within the atlas
	uint16 YOffset;		// Y pixel offset of this frame within the atlas
	uint16 Width;		// Frame width in pixels
	uint16 Height;		// Frame height in pixels
	uint8  Index;		// Frame index (0-based)
};

//
// TEX file class. Loads, saves, and converts Trials HD texture files.
//
// The raw pixel data is preserved as-is (tiled, big-endian) for byte-accurate round-tripping.
// Use GetLinearPixelData() to obtain untiled, byte-swapped data suitable for PC use.
//
class Tex
{
public:
	//
	// File version. 4 = T4X (standard), 3 = T3X (legacy).
	//

	uint8 Version = 4;

	//
	// Total texture dimensions in pixels.
	// For sprite sheets, this is the full atlas size (not individual frame size).
	//

	uint16 Width  = 0;
	uint16 Height = 0;

	//
	// Pixel format (see TexFormat namespace).
	//

	uint8 Format = TexFormat::DXT5;

	//
	// Number of mipmap levels (including the base level).
	// Mips are generated down to 4×4 pixels for DXT formats.
	//

	uint8 MipCount = 0;

	//
	// Frame descriptors. For static textures this contains exactly one entry.
	// For animated sprite sheets, one entry per frame.
	//

	std::vector<TexFrame> Frames;

	//
	// Content-dependent trailing byte (T4X only). Varies per texture type.
	// Combine (diffuse) textures typically have 0xFE or 0xFF.
	//

	uint8 ContentByte = 0xFE;

	//
	// Raw pixel data as stored in the file (Xbox 360 tiled layout, big-endian byte order).
	// Includes the full mipmap chain for all levels, stored contiguously.
	//

	std::vector<uint8> PixelData;

	//
	// Loads a .tex file from disk.
	// Returns true on success, false on failure (with error logged to stderr).
	//

	bool Load(const std::string& FilePath);

	//
	// Saves the texture back to a binary .tex file.
	// Returns true on success, false on failure.
	//

	bool Save(const std::string& FilePath) const;

	//
	// Exports to a DDS file. The pixel data is untiled and byte-swapped for PC use.
	// Returns true on success, false on failure.
	//

	bool ExportDDS(const std::string& FilePath) const;

	//
	// Imports from a DDS file. The pixel data is byte-swapped and tiled for Xbox 360.
	// Returns true on success, false on failure.
	//

	bool ImportDDS(const std::string& FilePath);

	//
	// Prints a summary of the texture file to stdout.
	//

	void PrintSummary() const;

	//
	// Returns the total header size in bytes (magic + fields + frame descriptors + trailing).
	//

	uint32 GetHeaderSize() const;

	//
	// Returns the total expected pixel data size (all mip levels combined).
	//

	uint32 GetPixelDataSize() const;

	//
	// Returns the pixel data size for a single mip level.
	// Level 0 is the base (largest) level.
	//

	uint32 GetMipLevelSize(uint32 Level) const;

	//
	// Returns the byte offset into PixelData where a given mip level begins.
	//

	uint32 GetMipLevelOffset(uint32 Level) const;

	//
	// Returns the dimensions (in pixels) of a given mip level.
	//

	void GetMipLevelDimensions(uint32 Level, uint32& OutWidth, uint32& OutHeight) const;

	//
	// Converts the pixel data from Xbox 360 tiled layout to linear row-major order,
	// with endian byte-swapping applied. Returns the converted data.
	// This is what you need to feed into a standard PC DXT decoder or write into a DDS file.
	//

	std::vector<uint8> GetLinearPixelData() const;

	//
	// Sets the pixel data from linear row-major PC data.
	// Applies endian byte-swapping and Xbox 360 tiling. Replaces PixelData.
	//

	void SetLinearPixelData(const std::vector<uint8>& LinearData);

	//
	// Calculates the expected mip count for the given dimensions and format.
	//

	static uint8 CalcMipCount(uint16 Width, uint16 Height, uint8 Format);

private:
	//
	// Untiles a single mip level from Xbox 360 tiled layout to linear order.
	//

	void _UntileMipLevel(const uint8* TiledData, uint8* LinearData,
						 uint32 WidthPixels, uint32 HeightPixels, uint8 Format) const;

	//
	// Tiles a single mip level from linear order to Xbox 360 tiled layout.
	//

	void _TileMipLevel(const uint8* LinearData, uint8* TiledData,
					   uint32 WidthPixels, uint32 HeightPixels, uint8 Format) const;

	//
	// Byte-swaps pixel data in-place (16-bit word swap for DXT, 32-bit for ARGB8).
	//

	static void _EndianSwap(uint8* Data, uint32 Size, uint8 Format);

	//
	// Computes the Xbox 360 tiled offset for a block/pixel at logical position (x, y).
	//

	static uint32 _XGAddress2DTiledOffset(uint32 x, uint32 y, uint32 Width, uint32 BytesPerBlock);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////