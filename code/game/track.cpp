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

    std::vector<uint8> RawData = _DecompressLZMA(CompressedBuffer);
    if (RawData.empty())
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to decompress track file: %s\n", FilePath.c_str());
        return false;
    }

    return _OBJ5_Parse(RawData);
}

// Saves the objects back into a playable .trk file.
bool Track::Save(const std::string& FilePath)
{
    std::vector<uint8> Obj5;

    Obj5.push_back('O'); Obj5.push_back('B'); Obj5.push_back('J'); Obj5.push_back('5');
    WriteBE16(Obj5, Objects.size());

    for (const auto& Obj : Objects)
    {
        Obj5.push_back(Obj.TypeID);
    }
    for (const auto& Obj : Objects)
    {
        WriteBE16(Obj5, Obj.Variant);
    }
    for (const auto& Obj : Objects)
    {
        WriteBEFloat(Obj5, Obj.X);
    }
    for (const auto& Obj : Objects)
    {
        WriteBEFloat(Obj5, Obj.Y);
    }
    for (const auto& Obj : Objects)
    {
        WriteBEFloat(Obj5, Obj.Z);
    }
    for (const auto& Obj : Objects)
    {
        Obj5.push_back(Obj.RotX);
    }
    for (const auto& Obj : Objects)
    {        
        Obj5.push_back(Obj.RotY);
    }
    for (const auto& Obj : Objects)
    {
        Obj5.push_back(Obj.RotZ);
    }
    for (const auto& Obj : Objects)
    {
        Obj5.push_back(Obj.ScaleOrExtra);
    }

    Obj5.insert(Obj5.end(), PreGlueData.begin(), PreGlueData.end());

    if (!Joints.empty())
    {
        Obj5.push_back('G'); Obj5.push_back('L'); Obj5.push_back('U'); Obj5.push_back('E');
        WriteBE16(Obj5, Joints.size());

        for (const auto& Joint : Joints)
        {
            WriteBE16(Obj5, Joint.ObjA);
            WriteBE16(Obj5, Joint.ObjB);
            WriteBE32(Obj5, Joint.ParamA);
            WriteBE32(Obj5, Joint.ParamB);
        }
    }

    Obj5.insert(Obj5.end(), PostGlueData.begin(), PostGlueData.end());

    std::vector<uint8> CompressedStream = _CompressLZMA(Obj5);
    if (CompressedStream.empty())
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to compress track data for saving.\n");
        return false;
    }

    uint32 RealUncompressedSize = Obj5.size();
    OriginalHeader[8] = (RealUncompressedSize >> 24) & 0xFF;
    OriginalHeader[9] = (RealUncompressedSize >> 16) & 0xFF;
    OriginalHeader[10] = (RealUncompressedSize >> 8) & 0xFF;
    OriginalHeader[11] = RealUncompressedSize & 0xFF;

    std::ofstream OutTrack(FilePath, std::ios::binary);
    if (!OutTrack)
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to open output track file for writing: %s\n", FilePath.c_str());
        return false;
    }

    OutTrack.write(reinterpret_cast<const char*>(OriginalHeader.data()), OriginalHeader.size());
    OutTrack.write(reinterpret_cast<char*>(CompressedStream.data()), 5);
    OutTrack.write(reinterpret_cast<char*>(CompressedStream.data()) + 13, CompressedStream.size() - 13);

    OutTrack.close();
    return true;
}

// Prints an entire summary of the track to the console.
void Track::PrintSummary() const 
{
    std::cout << "[+] Extracted " << Objects.size() << " Objects and " << Joints.size() << " Joints.\n\n";
    
    std::cout << "--- Objects ---\n";
    for(size i = 0; i < Objects.size(); i++) 
    {
        std::cout << "ID: " << i << " | Type: " << (int32)Objects[i].TypeID 
                  << " | Pos: (" << std::fixed << std::setprecision(2) << Objects[i].X << ", " 
                  << Objects[i].Y << ", " << Objects[i].Z << ")\n";
    }

    std::cout << "\n--- Joints ---\n";
    for(size i = 0; i < Joints.size(); i++) 
    {
        std::cout << "Joint: " << i << " | Connects Obj " << Joints[i].ObjA 
                  << " to Obj " << Joints[i].ObjB << "\n";
    }
}

// Exports the track data to a human readable XML format. This is used for editing tracks.
bool Track::ExportXML(const std::string& FilePath) const
{
    xmlDocPtr Doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr Root = xmlNewNode(NULL, BAD_CAST "TrialsTrack");
    xmlDocSetRootElement(Doc, Root);

    xmlNewChild(Root, NULL, BAD_CAST "Header", BAD_CAST ToHex(OriginalHeader).c_str());
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
        xmlNewProp(Node, BAD_CAST "p1", BAD_CAST std::to_string(Jt.ParamA).c_str());
        xmlNewProp(Node, BAD_CAST "p2", BAD_CAST std::to_string(Jt.ParamB).c_str());
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
    if (Doc == NULL) return false;

    xmlNodePtr Root = xmlDocGetRootElement(Doc);

    Objects.clear();
    Joints.clear();

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
            OriginalHeader = FromHex(reinterpret_cast<char*>(Content));
            xmlFree(Content);
        }
        else if (NodeName == "PreGlue") 
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            PreGlueData = FromHex(reinterpret_cast<char*>(Content));
            xmlFree(Content);
        }
        else if (NodeName == "PostGlue") 
        {
            xmlChar* Content = xmlNodeGetContent(CurNode);
            PostGlueData = FromHex(reinterpret_cast<char*>(Content));
            xmlFree(Content);
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
                Jt.ParamA = std::stoul(GetPropStr(JtNode, "p1"));
                Jt.ParamB = std::stoul(GetPropStr(JtNode, "p2"));
                Joints.push_back(Jt);
            }
        }
    }

    xmlFreeDoc(Doc);
    xmlCleanupParser();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Decompresses the given LZMA compressed data and returns the decompressed data.
std::vector<uint8> Track::_DecompressLZMA(const std::vector<uint8>& FileData)
{
    if (FileData.size() < 24)
    {
        std::fprintf(stderr, "[Error] [track.cpp] File is too small to contain valid track header.\n");
        return {};
    }

    std::span<const uint8> BufferSpan(FileData);
    uint32 ExpectedSize = ReadBE32(BufferSpan, 8);

    size LzmaOffset = 0;
    for (size i = 16; i < std::min<size>(128, FileData.size() - 13); i++)
    {
        if (FileData[i] == 0x5D && FileData[i + 1] == 0x00 && FileData[i + 2] == 0x00)
        {
            LzmaOffset = i;
            break;
        }
    }

    if (LzmaOffset == 0)
    {
        std::fprintf(stderr, "[Error] [track.cpp] Could not find LZMA property byte (0x5D).\n");
        return {};
    }

    // Capture the original .trk header.
    OriginalHeader.assign(FileData.begin(), FileData.begin() + LzmaOffset);
    std::vector<uint8> SynthBuffer;
    SynthBuffer.reserve(13 + FileData.size() - LzmaOffset - 13);
    for (int32 i = 0; i < 5; i++)
    {
        SynthBuffer.push_back(FileData[LzmaOffset + i]);
    }

    uint64 UncompSize64 = ExpectedSize;
    for (int32 i = 0; i < 8; i++)
    {
        SynthBuffer.push_back((UncompSize64 >> (i * 8)) & 0xFF);
    }
    
    for (size i = LzmaOffset + 13; i < FileData.size(); i++)
    {
        SynthBuffer.push_back(FileData[i]);
    }

    std::vector<uint8> UncompressedData(ExpectedSize);

    lzma_stream strm = LZMA_STREAM_INIT;
    if (lzma_alone_decoder(&strm, uint64_max) != LZMA_OK)
    {
        std::fprintf(stderr, "[Error] [track.cpp] Failed to initialize LZMA decoder.\n");
        return {};
    }

    strm.next_in = SynthBuffer.data();
    strm.avail_in = SynthBuffer.size();
    strm.next_out = UncompressedData.data();
    strm.avail_out = UncompressedData.size();

    lzma_ret ret = lzma_code(&strm, LZMA_RUN);

    size ActualWritten = ExpectedSize - strm.avail_out;
    UncompressedData.resize(ActualWritten);

    if (ret != LZMA_OK && ret != LZMA_STREAM_END && ret != LZMA_BUF_ERROR)
    {
        std::fprintf(stderr, "[Error] [track.cpp] LZMA decompression failed with error code %d.\n", ret);
        lzma_end(&strm);
        return {};
    }

    lzma_end(&strm);
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
        std::fprintf(stderr, "[Error] [track.cpp] Failed to initialize LZMA encoder.\n");
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

// Finds a chunk with the given tag in the given buffer and returns its offset. Returns 0 if not found.
size Track::_FindChunk(const std::span<const uint8> Buffer, const std::string& Tag) const
{
    if (Buffer.size() < 4)
    {
        return 0;
    }
    
    for (size i = 0; i < Buffer.size() - 4; i++)
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

    if (RawData.size() < 8 || BufferSpan[0] != 'O' || BufferSpan[1] != 'B' || BufferSpan[2] != 'J' || BufferSpan[3] != '5')
    {
        std::fprintf(stderr, "[Error] [track.cpp] Invalid OBJ5 data: Missing magic header bytes.\n");
        return false;
    }

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
    if (GlueOffset == 0)
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
