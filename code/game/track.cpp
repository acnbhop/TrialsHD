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
#include <libxml/parser.h>
#include <libxml/tree.h>

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
    xmlDocPtr Doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr Root = xmlNewNode(NULL, BAD_CAST "TrialsTrack");
    xmlDocSetRootElement(Doc, Root);

    xmlNewChild(Root, NULL, BAD_CAST "Header", BAD_CAST ToHex(OriginalHeader).c_str());
    xmlNewChild(Root, NULL, BAD_CAST "CompMethod", BAD_CAST Compression.c_str());
    xmlNewChild(Root, NULL, BAD_CAST "LzmaStripped", BAD_CAST (LzmaStripped ? "1" : "0"));
    xmlNewChild(Root, NULL, BAD_CAST "PayloadType", BAD_CAST PayloadType.c_str());

    if (PayloadType == "OBJ5")
    {
        xmlNewChild(Root, NULL, BAD_CAST "PreGlue", BAD_CAST ToHex(PreGlueData).c_str());
        xmlNewChild(Root, NULL, BAD_CAST "PostGlue", BAD_CAST ToHex(PostGlueData).c_str());

        xmlNodePtr ObjsNode = xmlNewChild(Root, NULL, BAD_CAST "Objects", NULL);
        for (const auto& Obj : Objects)
        {
            xmlNodePtr Node = xmlNewChild(ObjsNode, NULL, BAD_CAST "Obj", NULL);
            xmlNewProp(Node, BAD_CAST "t", BAD_CAST std::to_string(Obj.TypeID).c_str());
            xmlNewProp(Node, BAD_CAST "v", BAD_CAST std::to_string(Obj.Variant).c_str());
            xmlNewProp(Node, BAD_CAST "x", BAD_CAST std::to_string(Obj.X).c_str());
            xmlNewProp(Node, BAD_CAST "y", BAD_CAST std::to_string(Obj.Y).c_str());
            xmlNewProp(Node, BAD_CAST "z", BAD_CAST std::to_string(Obj.Z).c_str());
            xmlNewProp(Node, BAD_CAST "rx", BAD_CAST std::to_string(Obj.RotX).c_str());
            xmlNewProp(Node, BAD_CAST "ry", BAD_CAST std::to_string(Obj.RotY).c_str());
            xmlNewProp(Node, BAD_CAST "rz", BAD_CAST std::to_string(Obj.RotZ).c_str());
            xmlNewProp(Node, BAD_CAST "s", BAD_CAST std::to_string(Obj.ScaleOrExtra).c_str());
        }

        xmlNodePtr JointsNode = xmlNewChild(Root, NULL, BAD_CAST "Joints", NULL);
        for (const auto& Jt : Joints)
        {
            xmlNodePtr Node = xmlNewChild(JointsNode, NULL, BAD_CAST "Joint", NULL);
            xmlNewProp(Node, BAD_CAST "a", BAD_CAST std::to_string(Jt.ObjA).c_str());
            xmlNewProp(Node, BAD_CAST "b", BAD_CAST std::to_string(Jt.ObjB).c_str());
            xmlNewProp(Node, BAD_CAST "pA", BAD_CAST std::to_string(Jt.ParamA).c_str());
            xmlNewProp(Node, BAD_CAST "pB", BAD_CAST std::to_string(Jt.ParamB).c_str());
        }
    }
    else if (PayloadType == "XML")
    {
        xmlNodePtr RawNode = xmlNewChild(Root, NULL, BAD_CAST "RawXML", NULL);
        xmlNodePtr CData = xmlNewCDataBlock(Doc, BAD_CAST RawXML.c_str(), RawXML.length());
        xmlAddChild(RawNode, CData);
    }
    else
    {
        // For Legacy OBJ formats and unknown data
        xmlNewChild(Root, NULL, BAD_CAST "RawPayload", BAD_CAST RawXML.c_str());
    }

    xmlSaveFormatFileEnc(FilePath.c_str(), Doc, "UTF-8", 1);
    xmlFreeDoc(Doc);
    xmlCleanupParser();

    return true;
}

// Imports the track data from a human readable XML format. This is used for editing tracks.
bool Track::ImportXML(const std::string& FilePath)
{
    xmlDocPtr Doc = xmlReadFile(FilePath.c_str(), NULL, 0);
    if (Doc == NULL)
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to parse XML file: %s\n", FilePath.c_str());
        return false;
    }

    xmlNodePtr Root = xmlDocGetRootElement(Doc);

    Objects.clear();
    Joints.clear();
    LzmaStripped = false;

    auto GetPropStr = [](xmlNodePtr Node, const char* PropName) -> std::string
    {
        xmlChar* Prop = xmlGetProp(Node, BAD_CAST PropName);
        if (Prop)
        {
            std::string Result = reinterpret_cast<char*>(Prop);
            xmlFree(Prop);
            return Result;
        }
        return "0";
    };

    for (xmlNodePtr CurNode = Root->children; CurNode; CurNode = CurNode->next)
    {
        if (CurNode->type != XML_ELEMENT_NODE) continue;

        std::string NodeName = reinterpret_cast<const char*>(CurNode->name);

        if (NodeName == "Header")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content) OriginalHeader = FromHex(reinterpret_cast<char*>(Content));
            xmlFree(Content);
        }
        else if (NodeName == "CompMethod")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content) Compression = reinterpret_cast<char*>(Content);
            xmlFree(Content);
        }
        else if (NodeName == "LzmaStripped")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content) LzmaStripped = (std::string(reinterpret_cast<char*>(Content)) == "1");
            xmlFree(Content);
        }
        else if (NodeName == "PayloadType")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content) PayloadType = reinterpret_cast<char*>(Content);
            xmlFree(Content);
        }
        else if (NodeName == "PreGlue")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content) PreGlueData = FromHex(reinterpret_cast<char*>(Content));
            xmlFree(Content);
        }
        else if (NodeName == "PostGlue")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content) PostGlueData = FromHex(reinterpret_cast<char*>(Content));
            xmlFree(Content);
        }
        else if (NodeName == "RawPayload")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content) RawXML = reinterpret_cast<char*>(Content);
            xmlFree(Content);
        }
        else if (NodeName == "RawXML")
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            if (Content)
            {
                RawXML = reinterpret_cast<char*>(Content);
                xmlFree(Content);
            }
        }
        else if (NodeName == "Objects")
        {
            for (xmlNodePtr ObjNode = CurNode->children; ObjNode; ObjNode = ObjNode->next)
            {
                if (ObjNode->type != XML_ELEMENT_NODE) continue;

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
            for (xmlNodePtr JtNode = CurNode->children; JtNode; JtNode = JtNode->next)
            {
                if (JtNode->type != XML_ELEMENT_NODE) continue;

                TrackJoint Jt;
                Jt.ObjA   = std::stoi(GetPropStr(JtNode, "a"));
                Jt.ObjB   = std::stoi(GetPropStr(JtNode, "b"));
                Jt.ParamA = std::stoul(GetPropStr(JtNode, "pA"));
                Jt.ParamB = std::stoul(GetPropStr(JtNode, "pB"));
                Joints.push_back(Jt);
            }
        }
    }

    xmlFreeDoc(Doc);
    xmlCleanupParser();
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

    std::cout << "--- First 5 Objects ---\n";
    for(size i = 0; i < std::min<size>(5, Objects.size()); i++)
    {
        std::cout << "ID: " << i << " | Type: " << (int32)Objects[i].TypeID
                  << " | Pos: (" << std::fixed << std::setprecision(2) << Objects[i].X << ", "
                  << Objects[i].Y << ", " << Objects[i].Z << ")\n";
    }

    std::cout << "\n--- First 5 Joints ---\n";
    for(size i = 0; i < std::min<size>(5, Joints.size()); i++)
    {
        std::cout << "Joint: " << i << " | Connects Obj " << Joints[i].ObjA
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
