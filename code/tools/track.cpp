#include "track.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <lzma.h>

namespace redlynx::game
{

std::vector<uint8_t> Track::_DecompressLZMA(const std::vector<uint8_t>& FileData) 
{
    if (FileData.size() < 24) 
    {
        std::cerr << "[-] File too small to contain TRK header.\n";
        return {};
    }

    std::span<const uint8_t> BufSpan(FileData);
    uint32_t ExpectedSize = tools::be_util::read_be32(BufSpan, 8);

    //
    // Locate the 0x5D LZMA property byte
    //
    
    size_t LzmaOffset = 0;
    for (size_t i = 16; i < std::min<size_t>(128, FileData.size() - 13); i++) 
    {
        if (FileData[i] == 0x5D && FileData[i+1] == 0x00 && FileData[i+2] == 0x00) 
        {
            LzmaOffset = i;
            break;
        }
    }

    if (LzmaOffset == 0) 
    {
        std::cerr << "[-] Could not find LZMA property byte (0x5D).\n";
        return {};
    }

    //
    // Synthesize the perfect 13-byte header
    //

    std::vector<uint8_t> SynthBuffer;
    SynthBuffer.reserve(13 + FileData.size() - LzmaOffset - 13);
    for (int i = 0; i < 5; i++) SynthBuffer.push_back(FileData[LzmaOffset + i]);
    
    uint64_t UncompSize64 = ExpectedSize;
    for (int i = 0; i < 8; i++) SynthBuffer.push_back((UncompSize64 >> (i * 8)) & 0xFF);
    for (size_t i = LzmaOffset + 13; i < FileData.size(); i++) SynthBuffer.push_back(FileData[i]);

    //
    // Decompress entirely in memory!
    //

    std::vector<uint8_t> UncompressedData(ExpectedSize);
    
    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_alone_decoder(&strm, UINT64_MAX) != LZMA_OK)
    {
        std::cerr << "[-] Failed to initialize LZMA decoder.\n";
        return {};
    }

    strm.next_in = SynthBuffer.data();
    strm.avail_in = SynthBuffer.size();
    strm.next_out = UncompressedData.data();
    strm.avail_out = UncompressedData.size();

    lzma_ret ret = lzma_code(&strm, LZMA_RUN);
    
    //
    // Calculate how many bytes were actually decompressed
    //

    size_t ActualWritten = ExpectedSize - strm.avail_out;
    UncompressedData.resize(ActualWritten);

    if (ret != LZMA_OK && ret != LZMA_STREAM_END && ret != LZMA_BUF_ERROR) 
    {
        std::cerr << "[-] LZMA decoding failed with error code: " << ret << "\n";
        lzma_end(&strm);
        return {};
    }

    lzma_end(&strm);
    return UncompressedData;
}

size_t Track::FindChunk(std::span<const uint8_t> Buffer, const std::string& Tag) const 
{
    for (size_t i = 0; i < Buffer.size() - 4; i++) 
    {
        if (Buffer[i] == Tag[0] && Buffer[i+1] == Tag[1] && 
            Buffer[i+2] == Tag[2] && Buffer[i+3] == Tag[3]) 
            {
            return i;
        }
    }
    return 0;
}

bool Track::_OBJ5Parse(const std::vector<uint8_t>& RawData) 
{
    std::span<const uint8_t> Buf(RawData);

    if (RawData.size() < 8 || Buf[0] != 'O' || Buf[1] != 'B' || Buf[2] != 'J' || Buf[3] != '5') 
    {
        std::cerr << "[-] Invalid or missing OBJ5 Magic Bytes.\n";
        return false;
    }

    //
    // Extract Objects (Structure of Arrays)
    //

    uint16_t ObjectCount = tools::be_util::read_be16(Buf, 4);
    
    size_t OffType    = 6;
    size_t OffVariant = OffType    + ObjectCount;
    size_t OffX       = OffVariant + (ObjectCount * 2);
    size_t OffY       = OffX       + (ObjectCount * 4);
    size_t OffZ       = OffY       + (ObjectCount * 4);
    size_t OffRotX    = OffZ       + (ObjectCount * 4);
    size_t OffRotY    = OffRotX    + ObjectCount;
    size_t OffRotZ    = OffRotY    + ObjectCount;
    size_t OffScale   = OffRotZ    + ObjectCount;

    Objects.reserve(ObjectCount);
    for (size_t i = 0; i < ObjectCount; i++) 
    {
        TrackObject Obj;
        Obj.TypeID       = Buf[OffType + i];
        Obj.Variant      = tools::be_util::read_be16(Buf, OffVariant + (i * 2));
        Obj.X            = tools::be_util::read_be_float(Buf, OffX + (i * 4));
        Obj.Y            = tools::be_util::read_be_float(Buf, OffY + (i * 4));
        Obj.Z            = tools::be_util::read_be_float(Buf, OffZ + (i * 4));
        Obj.RotX         = Buf[OffRotX + i];
        Obj.RotY         = Buf[OffRotY + i];
        Obj.RotZ         = Buf[OffRotZ + i];
        Obj.ScaleOrExtra = Buf[OffScale + i];
        Objects.push_back(Obj);
    }

    //
    // Extract Joints (GLUE Chunk - Array of Structures)
    //

    size_t GlueOffset = FindChunk(Buf, "GLUE");
    if (GlueOffset != 0) 
    {
        uint16_t ConstraintCount = tools::be_util::read_be16(Buf, GlueOffset + 4);
        size_t DataStart = GlueOffset + 6;
        size_t JointStride = 12;

        Joints.reserve(ConstraintCount);
        for (size_t i = 0; i < ConstraintCount; i++) 
        {
            size_t Current = DataStart + (i * JointStride);
            if (Current + JointStride > RawData.size()) break;

            TrackJoint Jt;
            Jt.ObjA   = tools::be_util::read_be16(Buf, Current);
            Jt.ObjB   = tools::be_util::read_be16(Buf, Current + 2);
            Jt.Param1 = tools::be_util::read_be32(Buf, Current + 4);
            Jt.Param2 = tools::be_util::read_be32(Buf, Current + 8);
            Joints.push_back(Jt);
        }
    }

    return true;
}

bool Track::Load(const std::string& Filepath) 
{
    std::ifstream Src(Filepath, std::ios::binary | std::ios::ate);
    if (!Src) {
        std::cerr << "[-] Error opening " << Filepath << "\n";
        return false;
    }

    size_t FileSize = Src.tellg();
    Src.seekg(0, std::ios::beg);
    std::vector<uint8_t> CompressedBuffer(FileSize);
    Src.read(reinterpret_cast<char*>(CompressedBuffer.data()), FileSize);
    Src.close();

    std::vector<uint8_t> RawData = _DecompressLZMA(CompressedBuffer);
    if (RawData.empty()) 
    {
        std::cerr << "[-] Failed to decompress LZMA data.\n";
        return false;
    }

    return _OBJ5Parse(RawData);
}

void Track::PrintSummary() const 
{
    std::cout << "[+] Extracted " << Objects.size() << " Objects and " << Joints.size() << " Joints.\n\n";
    
    std::cout << "--- First 5 Objects ---\n";
    for(size_t i = 0; i < std::min<size_t>(5, Objects.size()); i++) 
    {
        std::cout << "ID: " << i << " | Type: " << (int)Objects[i].TypeID 
                  << " | Pos: (" << std::fixed << std::setprecision(2) << Objects[i].X << ", " 
                  << Objects[i].Y << ", " << Objects[i].Z << ")\n";
    }

    std::cout << "\n--- First 5 Joints ---\n";
    for(size_t i = 0; i < std::min<size_t>(5, Joints.size()); i++) 
    {
        std::cout << "Joint: " << i << " | Connects Obj " << Joints[i].ObjA 
                  << " to Obj " << Joints[i].ObjB << "\n";
    }
}

} // namespace redlynx::game

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <track_file.trk>\n";
        return EXIT_FAILURE;
    }

    redlynx::game::Track Track;
    if (Track.Load(argv[1])) {
        Track.PrintSummary();
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
