//
// track.cpp: Definition of game track.
//

// File header
#include "track.hpp"

// Shared headers
#include "shared/util.hpp"

// Standard headers
#include <cstdio>
#include <iostream>
#include <fstream>

// External headers
#include <lzma.h>
#include <zlib.h>
#include <tinyxml2.h>

namespace redlynx::game
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Loads a .trk file, decompresses it and parses the track data.
bool Track::Load(const std::string& FilePath)
{
    std::ifstream TrackFile(FilePath, std::ios::binary | std::ios::ate);
    if (!TrackFile)
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to open track file: %s\n", FilePath.c_str());
        return false;
    }

    size FileSize = TrackFile.tellg();
    TrackFile.seekg(0, std::ios::beg);
    std::vector<uint8> CompressedBuffer(FileSize);
    TrackFile.read(reinterpret_cast<char*>(CompressedBuffer.data()), FileSize);
    TrackFile.close();

    std::vector<uint8> RawData = _Decompress(CompressedBuffer);
    if (RawData.empty())
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to decompress track file: %s\n", FilePath.c_str());
        return false;
    }

    // Determine what kind of uncompressed payload we are looking at
    if (RawData.size() >= 4 && RawData[0] == static_cast<uint8>('O') && RawData[1] == static_cast<uint8>('B') &&
        RawData[2] == static_cast<uint8>('J'))
    {
        char Version = static_cast<char>(RawData[3]);
        if (Version == '5')
        {
            PayloadType = "OBJ5";
            return _OBJ5_Parse(RawData);
        }
        else
        {
            // Legacy Map Format (OBJ1, OBJ2, OBJ4).
            //
            // Quarantine to Hex so it can be safely repacked.
            PayloadType = std::string("OBJ") + Version;
            RawXML = ToHex(RawData);
            return true;
        }
    }
    else if (RawData.size() >= 4 && RawData[0] == static_cast<uint8>('<'))
    {
        // RedLynx packed raw XML directly into the compressed payload for whatever reason.
        PayloadType = "XML";
        RawXML.assign(RawData.begin(), RawData.end());
        return true;
    }
    else
    {
        // Absolute unknown payload. Backup safely.
        PayloadType = "RAW";
        RawXML = ToHex(RawData);
        return true;
    }
}

// Saves the objects back into a playable .trk file.
bool Track::Save(const std::string& FilePath)
{
    std::vector<uint8> Payload;

    if (PayloadType == "OBJ5")
    {
        Payload.push_back('O'); Payload.push_back('B'); Payload.push_back('J'); Payload.push_back('5');
        WriteBE16(Payload, Objects.size());

        for (const auto& Obj : Objects) Payload.push_back(Obj.TypeID);
        for (const auto& Obj : Objects) WriteBE16(Payload, Obj.Variant);
        for (const auto& Obj : Objects) WriteBEFloat(Payload, Obj.X);
        for (const auto& Obj : Objects) WriteBEFloat(Payload, Obj.Y);
        for (const auto& Obj : Objects) WriteBEFloat(Payload, Obj.Z);
        for (const auto& Obj : Objects) Payload.push_back(Obj.RotX);
        for (const auto& Obj : Objects) Payload.push_back(Obj.RotY);
        for (const auto& Obj : Objects) Payload.push_back(Obj.RotZ);
        for (const auto& Obj : Objects) Payload.push_back(Obj.ScaleOrExtra);

        Payload.insert(Payload.end(), PreGlueData.begin(), PreGlueData.end());

        if (!Joints.empty())
        {
            Payload.push_back('G'); Payload.push_back('L'); Payload.push_back('U'); Payload.push_back('E');
            WriteBE16(Payload, Joints.size());

            for (const auto& Joint : Joints)
            {
                WriteBE16(Payload, Joint.ObjA);
                WriteBE16(Payload, Joint.ObjB);
                WriteBE32(Payload, Joint.ParamA);
                WriteBE32(Payload, Joint.ParamB);
            }
        }

        Payload.insert(Payload.end(), PostGlueData.begin(), PostGlueData.end());
    }
    else if (PayloadType == "XML")
    {
        Payload.assign(RawXML.begin(), RawXML.end());
    }
    else
    {
        // Recover legacy OBJ or RAW formats from Hex String
        Payload = FromHex(RawXML);
    }

    // Compress the payload
    std::vector<uint8> CompressedStream;
    if (Compression == "LZMA") CompressedStream = _CompressLZMA(Payload);
    else if (Compression == "ZLIB") CompressedStream = _CompressZLIB(Payload);
    else CompressedStream = Payload; // NONE

    if (CompressedStream.empty()) return false;

    std::ofstream OutTrack(FilePath, std::ios::binary);
    if (!OutTrack) return false;

    // Write original wrapper
    if (!OriginalHeader.empty())
    {
        if (Compression == "LZMA")
        {
            uint32 Magic = OriginalHeader.size() >= 4 ? ReadBE32(std::span<const uint8>(OriginalHeader), 0) : 0;

            if (LzmaStripped && Magic == 0xDEADBABE && OriginalHeader.size() >= 12)
            {
                // Inject the updated true payload size
                uint32 RealUncompressedSize = Payload.size();
                OriginalHeader[8] = (RealUncompressedSize >> 24) & 0xFF;
                OriginalHeader[9] = (RealUncompressedSize >> 16) & 0xFF;
                OriginalHeader[10] = (RealUncompressedSize >> 8) & 0xFF;
                OriginalHeader[11] = RealUncompressedSize & 0xFF;

                // Strip the 8-byte LZMA uncompressed size to match DEADBABE format
                OutTrack.write(reinterpret_cast<const char*>(OriginalHeader.data()), OriginalHeader.size());
                OutTrack.write(reinterpret_cast<const char*>(CompressedStream.data()), 5);
                OutTrack.write(reinterpret_cast<const char*>(CompressedStream.data()) + 13, CompressedStream.size() - 13);
            }
            else
            {
                // Keep entire 13-byte standard LZMA header
                OutTrack.write(reinterpret_cast<const char*>(OriginalHeader.data()), OriginalHeader.size());
                OutTrack.write(reinterpret_cast<const char*>(CompressedStream.data()), CompressedStream.size());
            }
        }
        else
        {
            OutTrack.write(reinterpret_cast<const char*>(OriginalHeader.data()), OriginalHeader.size());
            OutTrack.write(reinterpret_cast<const char*>(CompressedStream.data()), CompressedStream.size());
        }
    }
    else
    {
        // No header available (e.g. standard ZLIB starts immediately)
        OutTrack.write(reinterpret_cast<const char*>(CompressedStream.data()), CompressedStream.size());
    }

    OutTrack.close();
    return true;
}

// Exports the track data to a human readable XML format. This is used for editing tracks.
bool Track::ExportXML(const std::string& FilePath) const
{
    tinyxml2::XMLDocument Doc;
    Doc.InsertEndChild(Doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\""));

    tinyxml2::XMLElement* Root = Doc.NewElement("TrialsTrack");
    Doc.InsertEndChild(Root);

    auto AddChildText = [&](tinyxml2::XMLElement* Parent, const char* Name, const std::string& Text) {
        tinyxml2::XMLElement* Element = Doc.NewElement(Name);
        Element->SetText(Text.c_str());
        Parent->InsertEndChild(Element);
    };

    AddChildText(Root, "Header", ToHex(OriginalHeader));
    AddChildText(Root, "CompMethod", Compression);
    AddChildText(Root, "LzmaStripped", LzmaStripped ? "1" : "0");
    AddChildText(Root, "PayloadType", PayloadType);

    if (PayloadType == "OBJ5")
    {
        AddChildText(Root, "PreGlue", ToHex(PreGlueData));
        AddChildText(Root, "PostGlue", ToHex(PostGlueData));

        tinyxml2::XMLElement* ObjsNode = Doc.NewElement("Objects");
        Root->InsertEndChild(ObjsNode);
        for (const auto& Obj : Objects)
        {
            tinyxml2::XMLElement* Node = Doc.NewElement("Obj");
            Node->SetAttribute("t", std::to_string(Obj.TypeID).c_str());
            Node->SetAttribute("v", std::to_string(Obj.Variant).c_str());
            Node->SetAttribute("x", std::to_string(Obj.X).c_str());
            Node->SetAttribute("y", std::to_string(Obj.Y).c_str());
            Node->SetAttribute("z", std::to_string(Obj.Z).c_str());
            Node->SetAttribute("rx", std::to_string(Obj.RotX).c_str());
            Node->SetAttribute("ry", std::to_string(Obj.RotY).c_str());
            Node->SetAttribute("rz", std::to_string(Obj.RotZ).c_str());
            Node->SetAttribute("s", std::to_string(Obj.ScaleOrExtra).c_str());
            ObjsNode->InsertEndChild(Node);
        }

        tinyxml2::XMLElement* JointsNode = Doc.NewElement("Joints");
        Root->InsertEndChild(JointsNode);
        for (const auto& Jt : Joints)
        {
            tinyxml2::XMLElement* Node = Doc.NewElement("Joint");
            Node->SetAttribute("a", std::to_string(Jt.ObjA).c_str());
            Node->SetAttribute("b", std::to_string(Jt.ObjB).c_str());
            Node->SetAttribute("pA", std::to_string(Jt.ParamA).c_str());
            Node->SetAttribute("pB", std::to_string(Jt.ParamB).c_str());
            JointsNode->InsertEndChild(Node);
        }
    }
    else if (PayloadType == "XML")
    {
        tinyxml2::XMLElement* RawNode = Doc.NewElement("RawXML");
        tinyxml2::XMLText* CData = Doc.NewText(RawXML.c_str());
        CData->SetCData(true);
        RawNode->InsertEndChild(CData);
        Root->InsertEndChild(RawNode);
    }
    else
    {
        // For Legacy OBJ formats and unknown data
        AddChildText(Root, "RawPayload", RawXML);
    }

    return Doc.SaveFile(FilePath.c_str()) == tinyxml2::XML_SUCCESS;
}

// Imports the track data from a human readable XML format. This is used for editing tracks.
bool Track::ImportXML(const std::string& FilePath)
{
    tinyxml2::XMLDocument Doc;
    if (Doc.LoadFile(FilePath.c_str()) != tinyxml2::XML_SUCCESS)
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to parse XML file: %s\n", FilePath.c_str());
        return false;
    }

    tinyxml2::XMLElement* Root = Doc.RootElement();
    if (!Root)
    {
        return false;
    }

    Objects.clear();
    Joints.clear();
    LzmaStripped = false;

    auto GetPropStr = [](tinyxml2::XMLElement* Node, const char* PropName) -> std::string
    {
        const char* Prop = Node->Attribute(PropName);
        if (Prop)
        {
            return std::string(Prop);
        }
        return "0";
    };

    for (tinyxml2::XMLElement* CurNode = Root->FirstChildElement(); CurNode; CurNode = CurNode->NextSiblingElement())
    {
        std::string NodeName = CurNode->Name();
        const char* ContentCStr = CurNode->GetText();
        std::string Content = ContentCStr ? ContentCStr : "";

        if (NodeName == "Header")
        {
            OriginalHeader = FromHex(Content);
        }
        else if (NodeName == "CompMethod")
        {
            Compression = Content;
        }
        else if (NodeName == "LzmaStripped")
        {
            LzmaStripped = (Content == "1");
        }
        else if (NodeName == "PayloadType")
        {
            PayloadType = Content;
        }
        else if (NodeName == "PreGlue")
        {
            PreGlueData = FromHex(Content);
        }
        else if (NodeName == "PostGlue")
        {
            PostGlueData = FromHex(Content);
        }
        else if (NodeName == "RawPayload")
        {
            RawXML = Content;
        }
        else if (NodeName == "RawXML")
        {
            RawXML = Content;
        }
        else if (NodeName == "Objects")
        {
            for (tinyxml2::XMLElement* ObjNode = CurNode->FirstChildElement("Obj"); ObjNode; ObjNode = ObjNode->NextSiblingElement("Obj"))
            {
                TrackObject Obj;
                Obj.TypeID       = std::stoi(GetPropStr(ObjNode, "t"));
                Obj.Variant      = std::stoi(GetPropStr(ObjNode, "v"));
                Obj.X            = std::stof(GetPropStr(ObjNode, "x"));
                Obj.Y            = std::stof(GetPropStr(ObjNode, "y"));
                Obj.Z            = std::stof(GetPropStr(ObjNode, "z"));
                Obj.RotX         = std::stoi(GetPropStr(ObjNode, "rx"));
                Obj.RotY         = std::stoi(GetPropStr(ObjNode, "ry"));
                Obj.RotZ         = std::stoi(GetPropStr(ObjNode, "rz"));
                Obj.ScaleOrExtra = std::stoi(GetPropStr(ObjNode, "s"));
                Objects.push_back(Obj);
            }
        }
        else if (NodeName == "Joints")
        {
            for (tinyxml2::XMLElement* JtNode = CurNode->FirstChildElement("Joint"); JtNode; JtNode = JtNode->NextSiblingElement("Joint"))
            {
                TrackJoint Jt;
                Jt.ObjA   = std::stoi(GetPropStr(JtNode, "a"));
                Jt.ObjB   = std::stoi(GetPropStr(JtNode, "b"));
                Jt.ParamA = std::stoul(GetPropStr(JtNode, "pA"));
                Jt.ParamB = std::stoul(GetPropStr(JtNode, "pB"));
                Joints.push_back(Jt);
            }
        }
    }

    return true;
}

// Prints an entire summary of the track to the console.
void Track::PrintSummary() const
{
    if (PayloadType == "XML")
    {
        std::cout << "[+] Track contains a raw XML payload (" << RawXML.size() << " bytes).\n";
        return;
    }
    else if (PayloadType != "OBJ5")
    {
        std::cout << "[+] Track contains Legacy/Raw Format: " << PayloadType << " (" << RawXML.size() / 2 << " bytes)\n";
        return;
    }

    std::cout << "[+] Extracted " << Objects.size() << " Objects and " << Joints.size() << " Joints.\n\n";

    std::cout << "--- Objects ---\n";
    for(size i = 0; i < Objects.size(); i++)
    {
        std::cout << "ID: " << i << "\t\t| Type: " << (int32)Objects[i].TypeID
                  << "\t\t| Pos: (" << std::fixed << std::setprecision(2) << Objects[i].X << ", "
                  << Objects[i].Y << ", " << Objects[i].Z << ")\n";
    }

    std::cout << "\n--- Joints ---\n";
    for(size i = 0; i < Joints.size(); i++)
    {
        std::cout << "Joint: " << i << "\t\t| Connects Obj " << Joints[i].ObjA
                  << " to Obj " << Joints[i].ObjB << "\n";
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Decompresses the given LZMA compressed data and returns the decompressed data.
std::vector<uint8> Track::_Decompress(const std::vector<uint8>& FileData)
{
    if (FileData.size() < 4) return {};

    size CompOffset = 0;
    Compression = "NONE";
    LzmaStripped = false;

    // Scan for compression streams dynamically
    for (size i = 0; i <= FileData.size() - 4; i++)
    {
        // ZLIB Magic Check
        if (FileData[i] == 0x78)
        {
            uint16 zhead = (static_cast<uint16>(FileData[i]) << 8) | FileData[i+1];
            if (zhead % 31 == 0)
            {
                Compression = "ZLIB";
                CompOffset = i;
                break;
            }
        }

        // LZMA Magic Check (0x5D properties byte)
        if (FileData[i] == 0x5D && FileData[i+1] == 0x00 && FileData[i+2] == 0x00)
        {
            Compression = "LZMA";
            CompOffset = i;
            break;
        }
    }

    if (Compression == "NONE")
    {
        std::fprintf(stderr, "[-] Could not find ZLIB or LZMA signature.\n");
        return {};
    }

    OriginalHeader.assign(FileData.begin(), FileData.begin() + CompOffset);

    std::vector<uint8> UncompressedData;

    if (Compression == "LZMA")
    {
        auto TryDecode = [](const std::vector<uint8>& synth_buf, std::vector<uint8>& out_data) -> bool
        {
            lzma_stream strm = LZMA_STREAM_INIT;
            if (lzma_alone_decoder(&strm, UINT64_MAX) != LZMA_OK) return false;

            strm.next_in = synth_buf.data();
            strm.avail_in = synth_buf.size();

            uint8 chunk[16384];
            bool success = true;
            while (true)
            {
                strm.next_out = chunk;
                strm.avail_out = sizeof(chunk);
                lzma_ret ret = lzma_code(&strm, LZMA_RUN);

                size have = sizeof(chunk) - strm.avail_out;
                out_data.insert(out_data.end(), chunk, chunk + have);

                if (ret == LZMA_STREAM_END) break;
                if (ret != LZMA_OK) { success = false; break; }
                if (strm.avail_out > 0 && strm.avail_in == 0) break;
            }
            lzma_end(&strm);
            return success;
        };

        std::vector<uint8> SynthBufferIntact(FileData.begin() + CompOffset, FileData.end());
        if (TryDecode(SynthBufferIntact, UncompressedData) && !UncompressedData.empty())
        {
            LzmaStripped = false;
        }
        else
        {
            UncompressedData.clear();
            std::vector<uint8> SynthBufferStripped;

            uint32 ExpectedSize = 1024 * 1024 * 10;
            if (OriginalHeader.size() >= 12 && OriginalHeader[0] == 0xDE && OriginalHeader[1] == 0xAD &&
                OriginalHeader[2] == 0xBA && OriginalHeader[3] == 0xBE)
            {
                ExpectedSize = ReadBE32(std::span<const uint8>(OriginalHeader), 8);
            }

            SynthBufferStripped.reserve(13 + FileData.size() - CompOffset - 5);
            for (int32 i = 0; i < 5; i++) SynthBufferStripped.push_back(FileData[CompOffset + i]);
            uint64 UncompSize64 = ExpectedSize;
            for (int32 i = 0; i < 8; i++) SynthBufferStripped.push_back((UncompSize64 >> (i * 8)) & 0xFF);
            for (size i = CompOffset + 5; i < FileData.size(); i++) SynthBufferStripped.push_back(FileData[i]);

            if (TryDecode(SynthBufferStripped, UncompressedData) && !UncompressedData.empty())
            {
                LzmaStripped = true;
            }
            else
            {
                std::fprintf(stderr, "[-] Failed to decode LZMA stream.\n");
                return {};
            }
        }
    }
    else if (Compression == "ZLIB")
    {
        z_stream strm = {};
        if (inflateInit(&strm) != Z_OK) return {};

        strm.next_in = (Bytef*)(FileData.data() + CompOffset);
        strm.avail_in = FileData.size() - CompOffset;

        uint8 chunk[16384];

        while (true)
        {
            strm.next_out = chunk;
            strm.avail_out = sizeof(chunk);
            int ret = inflate(&strm, Z_NO_FLUSH);

            size have = sizeof(chunk) - strm.avail_out;
            UncompressedData.insert(UncompressedData.end(), chunk, chunk + have);

            if (ret == Z_STREAM_END) break;
            if (ret != Z_OK && ret != Z_BUF_ERROR) break;
            if (strm.avail_out > 0 && strm.avail_in == 0) break;
        }

        inflateEnd(&strm);
    }

    return UncompressedData;
}

// Compresses the given data using LZMA and returns the compressed data.
std::vector<uint8> Track::_CompressLZMA(const std::vector<uint8>& FileData)
{
    lzma_options_lzma opt;
    lzma_lzma_preset(&opt, 5);

    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_alone_encoder(&strm, &opt) != LZMA_OK)
    {
        std::fprintf(stderr, "[-] Failed to initialize LZMA encoder.\n");
        return {};
    }

    std::vector<uint8> OutData(FileData.size() + 1024);
    strm.next_in = FileData.data();
    strm.avail_in = FileData.size();
    strm.next_out = OutData.data();
    strm.avail_out = OutData.size();

    lzma_code(&strm, LZMA_FINISH);
    OutData.resize(OutData.capacity() - strm.avail_out);
    lzma_end(&strm);
    return OutData;
}

// Compresses the given data using zlib and returns the compressed data.
std::vector<uint8> Track::_CompressZLIB(const std::vector<uint8>& FileData)
{
    std::vector<uint8> OutData(FileData.size() + 1024);

    z_stream strm = {};
    if (deflateInit(&strm, Z_BEST_COMPRESSION) != Z_OK)
    {
        std::fprintf(stderr, "[-] Failed to initialize zlib encoder.\n");
        return {};
    }

    strm.next_in = (Bytef*)FileData.data();
    strm.avail_in = FileData.size();
    strm.next_out = OutData.data();
    strm.avail_out = OutData.size();

    deflate(&strm, Z_FINISH);
    OutData.resize(OutData.capacity() - strm.avail_out);
    deflateEnd(&strm);

    return OutData;
}

// Finds a chunk with the given tag in the given buffer and returns its offset. Returns 0 if not found.
size Track::_FindChunk(const std::span<const uint8> Buffer, const std::string& Tag) const
{
    if (Buffer.size() < 4)
    {
        return 0;
    }

    for (size i = 0; i <= Buffer.size() - 4; i++)
    {
        if (Buffer[i] == Tag[0] && Buffer[i + 1] == Tag[1] && Buffer[i + 2] == Tag[2] && Buffer[i + 3] == Tag[3])
        {
            return i;
        }
    }

    return 0;
}

// Parses the given decompressed track data and fills the Objects and Joints vectors. Returns true on success.
bool Track::_OBJ5_Parse(const std::vector<uint8>& RawData)
{
    std::span<const uint8> BufferSpan(RawData);

    uint16 ObjectCount = ReadBE16(BufferSpan, 4);

    size OffType = 6;
    size OffVariant = OffType + ObjectCount;
    size OffX = OffVariant + (ObjectCount * 2);
    size OffY = OffX + (ObjectCount * 4);
    size OffZ = OffY + (ObjectCount * 4);
    size OffRotX = OffZ + (ObjectCount * 4);
    size OffRotY = OffRotX + ObjectCount;
    size OffRotZ = OffRotY + ObjectCount;
    size OffScale = OffRotZ + ObjectCount;
    size EndOfObjects = OffScale + ObjectCount;

    Objects.reserve(ObjectCount);
    for (size i = 0; i < ObjectCount; i++)
    {
        TrackObject Object;
        Object.TypeID = BufferSpan[OffType + i];
        Object.Variant = ReadBE16(BufferSpan, OffVariant + (i * 2));
        Object.X = ReadBEFloat(BufferSpan, OffX + (i * 4));
        Object.Y = ReadBEFloat(BufferSpan, OffY + (i * 4));
        Object.Z = ReadBEFloat(BufferSpan, OffZ + (i * 4));
        Object.RotX = BufferSpan[OffRotX + i];
        Object.RotY = BufferSpan[OffRotY + i];
        Object.RotZ = BufferSpan[OffRotZ + i];
        Object.ScaleOrExtra = BufferSpan[OffScale + i];
        Objects.push_back(Object);
    }

    size GlueOffset = _FindChunk(BufferSpan, "GLUE");
    if (GlueOffset != 0)
    {
        PreGlueData.assign(RawData.begin() + EndOfObjects, RawData.begin() + GlueOffset);

        uint16 ConstraintCount = ReadBE16(BufferSpan, GlueOffset + 4);
        size DataStart = GlueOffset + 6;
        size JointStride = 12;

        Joints.reserve(ConstraintCount);
        for (size i = 0; i < ConstraintCount; i++)
        {
            size Current = DataStart + (i * JointStride);
            if (Current + JointStride > RawData.size())
            {
                break;
            }

            TrackJoint TJ;

            TJ.ObjA = ReadBE16(BufferSpan, Current);
            TJ.ObjB = ReadBE16(BufferSpan, Current + 2);
            TJ.ParamA = ReadBE32(BufferSpan, Current + 4);
            TJ.ParamB = ReadBE32(BufferSpan, Current + 8);
            Joints.push_back(TJ);
        }

        size EndOfGlue = DataStart + (ConstraintCount * JointStride);
        PostGlueData.assign(RawData.begin() + EndOfGlue, RawData.end());
    }
    else
    {
        PreGlueData.assign(RawData.begin() + EndOfObjects, RawData.end());
    }

    return true;
}

} // namespace redlynx::game
