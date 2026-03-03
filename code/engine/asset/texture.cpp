//
// texture.cpp: Trials HD texture file (.tex) parser.
//

// File header
#include "texture.hpp"

// External headers
#include <dds/dds.hpp>

// Standard headers
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

// Loads a .tex file from disk.
bool Tex::Load(const std::string& FilePath)
{
	std::ifstream File(FilePath, std::ios::binary | std::ios::ate);
	if (!File)
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Failed to open TEX file: %s\n", FilePath.c_str());
		return false;
	}

	size RawSize = File.tellg();
	File.seekg(0, std::ios::beg);

	if (RawSize < 10)
	{
		std::fprintf(stderr, "[Error] [texture.cpp] File too small to be a valid TEX: %s\n", FilePath.c_str());
		return false;
	}

	// Read the entire file
	std::vector<uint8> Data(RawSize);
	File.read(reinterpret_cast<char*>(Data.data()), RawSize);
	File.close();

	// Parse magic (3 bytes)
	if (Data[0] == 'T' && Data[1] == '4' && Data[2] == 'X')
	{
		Version = 4;
	}
	else if (Data[0] == 'T' && Data[1] == '3' && Data[2] == 'X')
	{
		Version = 3;
	}
	else
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Invalid magic bytes in TEX file: %s (got 0x%02x 0x%02x 0x%02x)\n",
					 FilePath.c_str(), Data[0], Data[1], Data[2]);
		return false;
	}

	// Parse base header fields (offsets 3-9)
	Width    = static_cast<uint16>(Data[3]) | (static_cast<uint16>(Data[4]) << 8);
	Height   = static_cast<uint16>(Data[5]) | (static_cast<uint16>(Data[6]) << 8);
	Format   = Data[7];
	MipCount = Data[8];

	uint8 FrameCount = Data[9];

	// Validate format
	if (Format != TexFormat::ARGB8 && Format != TexFormat::DXT1 && Format != TexFormat::DXT5)
	{
		std::fprintf(stderr, "[Warning] [texture.cpp] Unknown pixel format 0x%02x in: %s\n", Format, FilePath.c_str());
	}

	// Validate dimensions
	if (Width == 0 || Height == 0)
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Invalid dimensions %ux%u in: %s\n", Width, Height, FilePath.c_str());
		return false;
	}

	// Handle frame count: T3X files may have frame count = 0 (meaning 1 frame, no descriptors)
	uint32 NumFrames = FrameCount;
	if (NumFrames == 0)
	{
		NumFrames = 1;
	}

	// Calculate expected header size
	uint32 TrailingBytes = (Version == 4) ? 2 : 1;
	uint32 ExpectedHeaderSize = 10 + NumFrames * 18 + TrailingBytes;

	if (RawSize < ExpectedHeaderSize)
	{
		std::fprintf(stderr, "[Error] [texture.cpp] File too small for header (%zu < %u): %s\n",
					 RawSize, ExpectedHeaderSize, FilePath.c_str());
		return false;
	}

	// Parse frame descriptors
	Frames.clear();
	Frames.resize(NumFrames);

	size Pos = 10;
	for (uint32 i = 0; i < NumFrames; i++)
	{
		TexFrame& Frame = Frames[i];

		// byte 0: reserved (always 0x00)
		// bytes 1-2: X offset (LE uint16)
		// bytes 3-4: Y offset (LE uint16)
		// bytes 5-8: reserved (zeros)
		// bytes 9-10: frame width (LE uint16)
		// bytes 11-12: frame height (LE uint16)
		// bytes 13-14: frame width duplicate (LE uint16)
		// bytes 15-16: frame height duplicate (LE uint16)
		// byte 17: frame index

		Frame.XOffset = static_cast<uint16>(Data[Pos + 1]) | (static_cast<uint16>(Data[Pos + 2]) << 8);
		Frame.YOffset = static_cast<uint16>(Data[Pos + 3]) | (static_cast<uint16>(Data[Pos + 4]) << 8);
		Frame.Width   = static_cast<uint16>(Data[Pos + 9]) | (static_cast<uint16>(Data[Pos + 10]) << 8);
		Frame.Height  = static_cast<uint16>(Data[Pos + 11]) | (static_cast<uint16>(Data[Pos + 12]) << 8);
		Frame.Index   = Data[Pos + 17];

		Pos += 18;
	}

	// Parse trailing bytes
	// Skip padding byte (always 0x00)
	Pos++; // padding byte

	if (Version == 4)
	{
		ContentByte = Data[Pos];
		Pos++;
	}
	else
	{
		ContentByte = 0;
	}

	// Everything after the header is pixel data
	if (Pos < RawSize)
	{
		PixelData.assign(Data.begin() + Pos, Data.end());
	}
	else
	{
		PixelData.clear();
	}

	// Validate pixel data size
	uint32 ExpectedDataSize = GetPixelDataSize();
	if (PixelData.size() != ExpectedDataSize)
	{
		std::fprintf(stderr, "[Warning] [texture.cpp] Pixel data size mismatch: got %zu, expected %u in: %s\n",
					 PixelData.size(), ExpectedDataSize, FilePath.c_str());
	}

	return true;
}

// Saves the texture back to a binary .tex file.
bool Tex::Save(const std::string& FilePath) const
{
	std::ofstream File(FilePath, std::ios::binary);
	if (!File)
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Failed to open output file: %s\n", FilePath.c_str());
		return false;
	}

	// Write magic
	if (Version == 3)
	{
		File.write("T3X", 3);
	}
	else
	{
		File.write("T4X", 3);
	}

	// Write width and height (LE uint16)
	uint8 Buf[2];

	Buf[0] = static_cast<uint8>(Width & 0xFF);
	Buf[1] = static_cast<uint8>((Width >> 8) & 0xFF);
	File.write(reinterpret_cast<const char*>(Buf), 2);

	Buf[0] = static_cast<uint8>(Height & 0xFF);
	Buf[1] = static_cast<uint8>((Height >> 8) & 0xFF);
	File.write(reinterpret_cast<const char*>(Buf), 2);

	// Write format, mip count, frame count
	File.put(static_cast<char>(Format));
	File.put(static_cast<char>(MipCount));

	uint8 FrameCount = static_cast<uint8>(Frames.size());
	if (Version == 3 && FrameCount == 1)
	{
		FrameCount = 0; // T3X convention: 0 means single frame
	}
	File.put(static_cast<char>(FrameCount));

	// Write frame descriptors
	uint32 NumFrames = static_cast<uint32>(Frames.size());
	if (NumFrames == 0) NumFrames = 1; // Always write at least one frame descriptor

	for (uint32 i = 0; i < NumFrames; i++)
	{
		const TexFrame& Frame = (i < Frames.size()) ? Frames[i] : TexFrame{0, 0, Width, Height, 0};

		uint8 FrameBlock[18];
		std::memset(FrameBlock, 0, 18);

		// byte 0: reserved
		FrameBlock[0] = 0x00;

		// bytes 1-2: X offset
		FrameBlock[1] = static_cast<uint8>(Frame.XOffset & 0xFF);
		FrameBlock[2] = static_cast<uint8>((Frame.XOffset >> 8) & 0xFF);

		// bytes 3-4: Y offset
		FrameBlock[3] = static_cast<uint8>(Frame.YOffset & 0xFF);
		FrameBlock[4] = static_cast<uint8>((Frame.YOffset >> 8) & 0xFF);

		// bytes 5-8: reserved (zeros)

		// bytes 9-10: frame width
		FrameBlock[9]  = static_cast<uint8>(Frame.Width & 0xFF);
		FrameBlock[10] = static_cast<uint8>((Frame.Width >> 8) & 0xFF);

		// bytes 11-12: frame height
		FrameBlock[11] = static_cast<uint8>(Frame.Height & 0xFF);
		FrameBlock[12] = static_cast<uint8>((Frame.Height >> 8) & 0xFF);

		// bytes 13-14: frame width (duplicate)
		FrameBlock[13] = FrameBlock[9];
		FrameBlock[14] = FrameBlock[10];

		// bytes 15-16: frame height (duplicate)
		FrameBlock[15] = FrameBlock[11];
		FrameBlock[16] = FrameBlock[12];

		// byte 17: frame index
		FrameBlock[17] = Frame.Index;

		File.write(reinterpret_cast<const char*>(FrameBlock), 18);
	}

	// Write trailing bytes
	File.put(0x00); // padding byte

	if (Version == 4)
	{
		File.put(static_cast<char>(ContentByte));
	}

	// Write pixel data
	if (!PixelData.empty())
	{
		File.write(reinterpret_cast<const char*>(PixelData.data()), PixelData.size());
	}

	File.close();
	return true;
}

// Exports to a DDS file.
bool Tex::ExportDDS(const std::string& FilePath) const
{
	if (Width == 0 || Height == 0 || MipCount == 0 || PixelData.empty())
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Cannot export empty texture to DDS: %s\n", FilePath.c_str());
		return false;
	}

	// Convert pixel data to linear, byte-swapped format
	std::vector<uint8> LinearData = GetLinearPixelData();

	// Build DDS file: magic (4 bytes) + FileHeader (124 bytes) + pixel data
	uint32_t Magic = dds::DdsMagicNumber::DDS;

	dds::FileHeader Header;
	std::memset(&Header, 0, sizeof(Header));

	Header.size   = sizeof(dds::FileHeader);
	Header.flags  = static_cast<dds::HeaderFlags>(
						dds::HeaderFlags::Caps | dds::HeaderFlags::Height |
						dds::HeaderFlags::Width | dds::HeaderFlags::PixelFormat);
	Header.height = Height;
	Header.width  = Width;
	Header.caps1  = 0x00001000; // DDSCAPS_TEXTURE

	if (MipCount > 1)
	{
		Header.flags = static_cast<dds::HeaderFlags>(Header.flags | dds::HeaderFlags::Mipmap);
		Header.mipmapCount = MipCount;
		Header.caps1 |= 0x00000008 | 0x00400000; // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
	}
	else
	{
		Header.mipmapCount = 1;
	}

	Header.pixelFormat.size = sizeof(dds::FilePixelFormat);

	if (Format == TexFormat::DXT1)
	{
		Header.flags = static_cast<dds::HeaderFlags>(Header.flags | dds::HeaderFlags::LinearSize);
		Header.pitch = std::max(1u, (static_cast<uint32>(Width) + 3) / 4) *
					   std::max(1u, (static_cast<uint32>(Height) + 3) / 4) * 8;
		Header.pixelFormat.flags = dds::PixelFormatFlags::FourCC;
		Header.pixelFormat.fourCC = dds::DdsMagicNumber::DXT1;
	}
	else if (Format == TexFormat::DXT5)
	{
		Header.flags = static_cast<dds::HeaderFlags>(Header.flags | dds::HeaderFlags::LinearSize);
		Header.pitch = std::max(1u, (static_cast<uint32>(Width) + 3) / 4) *
					   std::max(1u, (static_cast<uint32>(Height) + 3) / 4) * 16;
		Header.pixelFormat.flags = dds::PixelFormatFlags::FourCC;
		Header.pixelFormat.fourCC = dds::DdsMagicNumber::DXT5;
	}
	else if (Format == TexFormat::ARGB8)
	{
		Header.flags = static_cast<dds::HeaderFlags>(Header.flags | dds::HeaderFlags::Pitch);
		Header.pitch = static_cast<uint32>(Width) * 4;
		Header.pixelFormat.flags = dds::PixelFormatFlags::RGBA;
		Header.pixelFormat.bitCount = 32;
		Header.pixelFormat.rBitMask = 0x00FF0000;
		Header.pixelFormat.gBitMask = 0x0000FF00;
		Header.pixelFormat.bBitMask = 0x000000FF;
		Header.pixelFormat.aBitMask = 0xFF000000;
	}
	else
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Unsupported format 0x%02x for DDS export: %s\n",
					 Format, FilePath.c_str());
		return false;
	}

	// Write DDS file
	std::ofstream File(FilePath, std::ios::binary);
	if (!File)
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Failed to open output DDS file: %s\n", FilePath.c_str());
		return false;
	}

	File.write(reinterpret_cast<const char*>(&Magic), sizeof(Magic));
	File.write(reinterpret_cast<const char*>(&Header), sizeof(Header));
	File.write(reinterpret_cast<const char*>(LinearData.data()), LinearData.size());
	File.close();

	return true;
}

// Imports from a DDS file.
bool Tex::ImportDDS(const std::string& FilePath)
{
	// Use the dds library to parse the DDS file
	dds::Image DdsImage;
	dds::ReadResult Result = dds::readFile(FilePath, &DdsImage);

	if (Result != dds::ReadResult::Success)
	{
		std::fprintf(stderr, "[Error] [texture.cpp] Failed to read DDS file (error %d): %s\n",
					 static_cast<int>(Result), FilePath.c_str());
		return false;
	}

	// Parse dimensions
	Width  = static_cast<uint16>(DdsImage.width);
	Height = static_cast<uint16>(DdsImage.height);

	// Parse format from DXGI format
	switch (DdsImage.format)
	{
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC1_TYPELESS:
			Format = TexFormat::DXT1;
			break;

		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
			Format = TexFormat::DXT5;
			break;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			Format = TexFormat::ARGB8;
			break;

		default:
			std::fprintf(stderr, "[Error] [texture.cpp] Unsupported DDS DXGI format %d in: %s\n",
						 static_cast<int>(DdsImage.format), FilePath.c_str());
			return false;
	}

	// Parse mip count
	MipCount = static_cast<uint8>(DdsImage.numMips);
	if (MipCount == 0) MipCount = 1;

	// Set up single frame
	Version = 4;
	Frames.clear();
	Frames.push_back({0, 0, Width, Height, 0});
	ContentByte = 0xFE;

	// Collect pixel data from mipmap spans into a contiguous linear buffer
	size TotalDataSize = 0;
	for (uint32 i = 0; i < DdsImage.numMips; ++i)
	{
		TotalDataSize += DdsImage.mipmaps[i].size();
	}

	std::vector<uint8> LinearData(TotalDataSize);
	size Offset = 0;
	for (uint32 i = 0; i < DdsImage.numMips; ++i)
	{
		std::memcpy(LinearData.data() + Offset, DdsImage.mipmaps[i].data(), DdsImage.mipmaps[i].size());
		Offset += DdsImage.mipmaps[i].size();
	}

	// Convert linear PC data to tiled Xbox 360 data
	SetLinearPixelData(LinearData);

	return true;
}

// Prints a summary of the texture file to stdout.
void Tex::PrintSummary() const
{
	std::cout << "[+] TEX v" << static_cast<int>(Version)
			  << " | " << Width << "x" << Height
			  << " | " << TexFormatToString(Format)
			  << " | Mips: " << static_cast<int>(MipCount)
			  << " | Frames: " << Frames.size()
			  << " | Data: " << PixelData.size() << " bytes\n\n";

	// Header info
	std::cout << "--- Header ---\n";
	std::cout << "  Magic: T" << static_cast<int>(Version) << "X\n";
	std::cout << "  Dimensions: " << Width << " x " << Height << "\n";
	std::cout << "  Format: 0x" << std::hex << std::setw(2) << std::setfill('0')
			  << static_cast<int>(Format) << std::dec << " (" << TexFormatToString(Format) << ")\n";
	std::cout << "  Mip Levels: " << static_cast<int>(MipCount) << "\n";
	std::cout << "  Frame Count: " << Frames.size() << "\n";
	std::cout << "  Header Size: " << GetHeaderSize() << " bytes\n";

	if (Version == 4)
	{
		std::cout << "  Content Byte: 0x" << std::hex << std::setw(2) << std::setfill('0')
				  << static_cast<int>(ContentByte) << std::dec << "\n";
	}
	std::cout << "\n";

	// Frame info
	if (!Frames.empty())
	{
		std::cout << "--- Frames (" << Frames.size() << ") ---\n";
		for (size i = 0; i < Frames.size(); i++)
		{
			const TexFrame& Frame = Frames[i];
			std::cout << "  [" << std::setw(2) << std::setfill(' ') << Frame.Index << "] "
					  << Frame.Width << "x" << Frame.Height
					  << " @ (" << Frame.XOffset << ", " << Frame.YOffset << ")\n";
		}
		std::cout << "\n";
	}

	// Mip chain info
	std::cout << "--- Mip Chain ---\n";
	uint32 TotalDataSize = 0;
	for (uint32 i = 0; i < MipCount; i++)
	{
		uint32 MipW, MipH;
		GetMipLevelDimensions(i, MipW, MipH);
		uint32 LevelSize = GetMipLevelSize(i);
		uint32 LevelOffset = GetMipLevelOffset(i);
		TotalDataSize += LevelSize;

		if (TexFormatIsCompressed(Format))
		{
			uint32 BlocksW = std::max(1u, MipW / 4);
			uint32 BlocksH = std::max(1u, MipH / 4);
			std::cout << "  Mip " << std::setw(2) << std::setfill(' ') << i
					  << ": " << std::setw(5) << MipW << "x" << std::setw(5) << std::left << MipH << std::right
					  << " (" << std::setw(4) << BlocksW << "x" << std::setw(4) << std::left << BlocksH << std::right
					  << " blocks)"
					  << " | " << std::setw(10) << LevelSize << " bytes"
					  << " @ offset " << LevelOffset << "\n";
		}
		else
		{
			std::cout << "  Mip " << std::setw(2) << std::setfill(' ') << i
					  << ": " << std::setw(5) << MipW << "x" << std::setw(5) << std::left << MipH << std::right
					  << " | " << std::setw(10) << LevelSize << " bytes"
					  << " @ offset " << LevelOffset << "\n";
		}
	}
	std::cout << "  Total: " << TotalDataSize << " bytes"
			  << " (actual: " << PixelData.size() << " bytes)\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Query Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns the total header size in bytes.
uint32 Tex::GetHeaderSize() const
{
	uint32 NumFrames = static_cast<uint32>(Frames.size());
	if (NumFrames == 0) NumFrames = 1;

	uint32 TrailingBytes = (Version == 4) ? 2 : 1;
	return 10 + NumFrames * 18 + TrailingBytes;
}

// Returns the total expected pixel data size (all mip levels combined).
uint32 Tex::GetPixelDataSize() const
{
	uint32 Total = 0;
	for (uint32 i = 0; i < MipCount; i++)
	{
		Total += GetMipLevelSize(i);
	}
	return Total;
}

// Returns the pixel data size for a single mip level.
uint32 Tex::GetMipLevelSize(uint32 Level) const
{
	uint32 MipW, MipH;
	GetMipLevelDimensions(Level, MipW, MipH);

	if (TexFormatIsCompressed(Format))
	{
		uint32 BlocksW = std::max(1u, MipW / 4);
		uint32 BlocksH = std::max(1u, MipH / 4);
		return BlocksW * BlocksH * TexFormatBytesPerBlock(Format);
	}
	else
	{
		return MipW * MipH * TexFormatBytesPerBlock(Format);
	}
}

// Returns the byte offset into PixelData where a given mip level begins.
uint32 Tex::GetMipLevelOffset(uint32 Level) const
{
	uint32 Offset = 0;
	for (uint32 i = 0; i < Level && i < MipCount; i++)
	{
		Offset += GetMipLevelSize(i);
	}
	return Offset;
}

// Returns the dimensions (in pixels) of a given mip level.
void Tex::GetMipLevelDimensions(uint32 Level, uint32& OutWidth, uint32& OutHeight) const
{
	OutWidth  = std::max(1u, static_cast<uint32>(Width) >> Level);
	OutHeight = std::max(1u, static_cast<uint32>(Height) >> Level);
}

// Calculates the expected mip count for the given dimensions and format.
uint8 Tex::CalcMipCount(uint16 InWidth, uint16 InHeight, uint8 InFormat)
{
	uint8 Count = 0;
	uint32 W = InWidth;
	uint32 H = InHeight;

	uint32 MinSize = TexFormatIsCompressed(InFormat) ? 4 : 1;

	while (W >= MinSize && H >= MinSize)
	{
		Count++;
		W >>= 1;
		H >>= 1;
	}

	return std::max(Count, static_cast<uint8>(1));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Pixel Data Conversion
////////////////////////////////////////////////////////////////////////////////////////////////////

// Converts from Xbox 360 tiled layout to linear row-major order with endian byte-swapping.
std::vector<uint8> Tex::GetLinearPixelData() const
{
	uint32 TotalSize = GetPixelDataSize();
	std::vector<uint8> LinearData(TotalSize);

	if (PixelData.size() < TotalSize)
	{
		std::fprintf(stderr, "[Warning] [texture.cpp] Pixel data smaller than expected (%zu < %u)\n",
					 PixelData.size(), TotalSize);
		return LinearData;
	}

	uint32 SrcOffset = 0;
	uint32 DstOffset = 0;

	for (uint32 Level = 0; Level < MipCount; Level++)
	{
		uint32 MipW, MipH;
		GetMipLevelDimensions(Level, MipW, MipH);
		uint32 LevelSize = GetMipLevelSize(Level);

		// Untile this mip level
		_UntileMipLevel(PixelData.data() + SrcOffset, LinearData.data() + DstOffset, MipW, MipH, Format);

		SrcOffset += LevelSize;
		DstOffset += LevelSize;
	}

	// Apply endian swap to the linear data
	_EndianSwap(LinearData.data(), TotalSize, Format);

	return LinearData;
}

// Sets the pixel data from linear row-major PC data.
void Tex::SetLinearPixelData(const std::vector<uint8>& LinearData)
{
	uint32 TotalSize = GetPixelDataSize();
	PixelData.resize(TotalSize);

	// Make a copy for byte-swapping
	std::vector<uint8> SwappedData = LinearData;

	// Apply endian swap (PC -> Xbox 360)
	uint32 CopySize = std::min(static_cast<uint32>(SwappedData.size()), TotalSize);
	_EndianSwap(SwappedData.data(), CopySize, Format);

	uint32 SrcOffset = 0;
	uint32 DstOffset = 0;

	for (uint32 Level = 0; Level < MipCount; Level++)
	{
		uint32 MipW, MipH;
		GetMipLevelDimensions(Level, MipW, MipH);
		uint32 LevelSize = GetMipLevelSize(Level);

		if (SrcOffset + LevelSize <= SwappedData.size())
		{
			// Tile this mip level
			_TileMipLevel(SwappedData.data() + SrcOffset, PixelData.data() + DstOffset, MipW, MipH, Format);
		}

		SrcOffset += LevelSize;
		DstOffset += LevelSize;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

// Computes the Xbox 360 tiled offset for a block/pixel at logical position (x, y).
// This implements the XGAddress2DTiledOffset algorithm for Xbox 360 GPU tiling.
//
// For DXT textures, x and y are in units of DXT blocks (not pixels).
// For uncompressed textures, x and y are in pixels.
//
// Width is the surface width in the same units (blocks for DXT, pixels for uncompressed).
// BytesPerElement is bytes per DXT block (8 or 16) or bytes per pixel (4).
uint32 Tex::_XGAddress2DTiledOffset(uint32 x, uint32 y, uint32 Width, uint32 BytesPerElement)
{
	// Aligned width in elements (align to 32 elements for proper tiling)
	uint32 AlignedWidth = (Width + 31) & ~31u;

	// Log2 of bytes per element
	uint32 LogBPE = 0;
	{
		uint32 BPE = BytesPerElement;
		while (BPE > 1)
		{
			LogBPE++;
			BPE >>= 1;
		}
	}

	// Macro tile dimensions depend on bytes per element:
	// For 8 bytes per element (DXT1): macro is 128 bytes wide, so 16 elements wide, 16 tall
	// For 16 bytes per element (DXT5): macro is 128 bytes wide, so 8 elements wide, 16 tall
	// For 4 bytes per element (ARGB8): macro is 128 bytes wide, so 32 elements wide, 16 tall

	// Macro tile pitch in elements
	uint32 MacroTilePitch = 128 >> LogBPE; // elements per macro tile row in X
	uint32 MacroTileHeight = 16;           // elements per macro tile in Y

	// Micro tile is always 8 bytes wide in each row, 8 rows tall in bytes
	// In elements: micro tile width = 8 / BytesPerElement? No, that's sub-element.
	// Actually for Xbox 360, the micro tile for thick formats is:
	// Micro tile: for DXT, it's a specific pattern.

	// The Xbox 360 tiling formula (simplified from XDK documentation):
	// Based on the XGAddress2DTiledOffset reference implementation.

	uint32 MacroTileX = x / MacroTilePitch;
	uint32 MacroTileY = y / MacroTileHeight;

	uint32 MacroPitch = AlignedWidth / MacroTilePitch;

	// Macro tile index
	uint32 MacroTileIdx = MacroTileY * MacroPitch + MacroTileX;

	// Macro tile offset in bytes
	uint32 MacroTileSize = MacroTilePitch * MacroTileHeight * BytesPerElement;
	uint32 MacroOffset = MacroTileIdx * MacroTileSize;

	// Position within macro tile
	uint32 LocalX = x % MacroTilePitch;
	uint32 LocalY = y % MacroTileHeight;

	// Micro tile within macro tile
	// Micro tile is 4 elements wide, 4 elements tall for thick, or varies.
	// For DXT with 8 bytes per block: micro tile = 4 wide x 4 tall
	// For DXT with 16 bytes per block: micro tile = 4 wide x 4 tall
	// For ARGB8 with 4 bytes per pixel: micro tile = 8 wide x 4 tall? (32 bytes per row)

	// Simplified approach: use a micro tile of 4x4 elements for DXT (8 or 16 bytes per block),
	// or 8x4 for uncompressed (4 bytes per pixel).
	// DXT blocks are 8 or 16 bytes; uncompressed pixels are 4 bytes.
	uint32 MicroW, MicroH;
	if (BytesPerElement >= 8)
	{
		// DXT formats (8 or 16 bytes per block): micro tile is 4 blocks wide x 4 blocks tall
		MicroW = 4;
		MicroH = 4;
	}
	else
	{
		// Uncompressed (4 bytes per pixel): micro tile is 8 pixels wide x 4 pixels tall
		MicroW = 8;
		MicroH = 4;
	}

	uint32 MicroTileX = LocalX / MicroW;
	uint32 MicroTileY = LocalY / MicroH;

	uint32 MicroTilesPerRow = MacroTilePitch / MicroW;

	// Micro tile index within macro tile (row-major)
	uint32 MicroTileIdx = MicroTileY * MicroTilesPerRow + MicroTileX;
	uint32 MicroTileSize = MicroW * MicroH * BytesPerElement;
	uint32 MicroOffset = MicroTileIdx * MicroTileSize;

	// Position within micro tile (linear, row-major)
	uint32 PixelX = LocalX % MicroW;
	uint32 PixelY = LocalY % MicroH;
	uint32 PixelOffset = (PixelY * MicroW + PixelX) * BytesPerElement;

	return MacroOffset + MicroOffset + PixelOffset;
}

// Untiles a single mip level from Xbox 360 tiled layout to linear order.
void Tex::_UntileMipLevel(const uint8* TiledData, uint8* LinearData,
						  uint32 WidthPixels, uint32 HeightPixels, uint8 Fmt) const
{
	uint32 BytesPerBlock = TexFormatBytesPerBlock(Fmt);

	if (TexFormatIsCompressed(Fmt))
	{
		// Work in units of DXT blocks
		uint32 BlocksW = std::max(1u, WidthPixels / 4);
		uint32 BlocksH = std::max(1u, HeightPixels / 4);
		uint32 LevelSize = BlocksW * BlocksH * BytesPerBlock;

		// For very small mip levels (single block), just copy directly
		if (BlocksW <= 1 && BlocksH <= 1)
		{
			std::memcpy(LinearData, TiledData, BytesPerBlock);
			return;
		}

		for (uint32 by = 0; by < BlocksH; by++)
		{
			for (uint32 bx = 0; bx < BlocksW; bx++)
			{
				uint32 LinearOffset = (by * BlocksW + bx) * BytesPerBlock;
				uint32 TiledOffset = _XGAddress2DTiledOffset(bx, by, BlocksW, BytesPerBlock);

				if (TiledOffset + BytesPerBlock <= LevelSize)
				{
					std::memcpy(LinearData + LinearOffset, TiledData + TiledOffset, BytesPerBlock);
				}
			}
		}
	}
	else
	{
		// Work in units of pixels
		uint32 LevelSize = WidthPixels * HeightPixels * BytesPerBlock;

		if (WidthPixels <= 1 && HeightPixels <= 1)
		{
			std::memcpy(LinearData, TiledData, BytesPerBlock);
			return;
		}

		for (uint32 py = 0; py < HeightPixels; py++)
		{
			for (uint32 px = 0; px < WidthPixels; px++)
			{
				uint32 LinearOffset = (py * WidthPixels + px) * BytesPerBlock;
				uint32 TiledOffset = _XGAddress2DTiledOffset(px, py, WidthPixels, BytesPerBlock);

				if (TiledOffset + BytesPerBlock <= LevelSize)
				{
					std::memcpy(LinearData + LinearOffset, TiledData + TiledOffset, BytesPerBlock);
				}
			}
		}
	}
}

// Tiles a single mip level from linear order to Xbox 360 tiled layout.
void Tex::_TileMipLevel(const uint8* LinearData, uint8* TiledData,
					    uint32 WidthPixels, uint32 HeightPixels, uint8 Fmt) const
{
	uint32 BytesPerBlock = TexFormatBytesPerBlock(Fmt);

	if (TexFormatIsCompressed(Fmt))
	{
		uint32 BlocksW = std::max(1u, WidthPixels / 4);
		uint32 BlocksH = std::max(1u, HeightPixels / 4);
		uint32 LevelSize = BlocksW * BlocksH * BytesPerBlock;

		if (BlocksW <= 1 && BlocksH <= 1)
		{
			std::memcpy(TiledData, LinearData, BytesPerBlock);
			return;
		}

		// Zero out the destination first since tiling may leave gaps
		std::memset(TiledData, 0, LevelSize);

		for (uint32 by = 0; by < BlocksH; by++)
		{
			for (uint32 bx = 0; bx < BlocksW; bx++)
			{
				uint32 LinearOffset = (by * BlocksW + bx) * BytesPerBlock;
				uint32 TiledOffset = _XGAddress2DTiledOffset(bx, by, BlocksW, BytesPerBlock);

				if (TiledOffset + BytesPerBlock <= LevelSize)
				{
					std::memcpy(TiledData + TiledOffset, LinearData + LinearOffset, BytesPerBlock);
				}
			}
		}
	}
	else
	{
		uint32 LevelSize = WidthPixels * HeightPixels * BytesPerBlock;

		if (WidthPixels <= 1 && HeightPixels <= 1)
		{
			std::memcpy(TiledData, LinearData, BytesPerBlock);
			return;
		}

		std::memset(TiledData, 0, LevelSize);

		for (uint32 py = 0; py < HeightPixels; py++)
		{
			for (uint32 px = 0; px < WidthPixels; px++)
			{
				uint32 LinearOffset = (py * WidthPixels + px) * BytesPerBlock;
				uint32 TiledOffset = _XGAddress2DTiledOffset(px, py, WidthPixels, BytesPerBlock);

				if (TiledOffset + BytesPerBlock <= LevelSize)
				{
					std::memcpy(TiledData + TiledOffset, LinearData + LinearOffset, BytesPerBlock);
				}
			}
		}
	}
}

// Byte-swaps pixel data in-place.
// DXT: swap every 16-bit word (2 bytes).
// ARGB8: swap every 32-bit word (4 bytes).
void Tex::_EndianSwap(uint8* Data, uint32 Size, uint8 Fmt)
{
	if (Fmt == TexFormat::ARGB8)
	{
		// 32-bit word swap (ARGB -> BGRA or vice versa)
		for (uint32 i = 0; i + 3 < Size; i += 4)
		{
			uint8 Tmp0 = Data[i];
			uint8 Tmp1 = Data[i + 1];
			Data[i]     = Data[i + 3];
			Data[i + 1] = Data[i + 2];
			Data[i + 2] = Tmp1;
			Data[i + 3] = Tmp0;
		}
	}
	else
	{
		// 16-bit word swap for DXT formats
		for (uint32 i = 0; i + 1 < Size; i += 2)
		{
			uint8 Tmp = Data[i];
			Data[i]     = Data[i + 1];
			Data[i + 1] = Tmp;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////