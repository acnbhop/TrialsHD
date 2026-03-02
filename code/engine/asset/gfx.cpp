//
// gfx.cpp: Graphics scene file (.gfx) parser.
//

// File header
#include "gfx.hpp"

// Shared headers
#include "shared/util.hpp"

// Standard headers
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

// External headers
#include <tinyxml2.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Loads a .gfx file from disk.
bool Gfx::Load(const std::string& FilePath)
{
	std::ifstream File(FilePath, std::ios::binary | std::ios::ate);
	if (!File)
	{
		std::fprintf(stderr, "[Error] [gfx.cpp] Failed to open GFX file: %s\n", FilePath.c_str());
		return false;
	}

	size RawSize = File.tellg();
	File.seekg(0, std::ios::beg);
	Data.resize(RawSize);
	File.read(reinterpret_cast<char*>(Data.data()), RawSize);
	File.close();

	if (RawSize < 104)
	{
		std::fprintf(stderr, "[Error] [gfx.cpp] File too small to be a valid GFX: %s\n", FilePath.c_str());
		return false;
	}

	// Parse version
	Version = ReadLE32(Data, 0);
	if (Version != 1)
	{
		std::fprintf(stderr, "[Warning] [gfx.cpp] Unexpected GFX version %u in: %s\n", Version, FilePath.c_str());
	}

	// Parse root node identity
	RootTypeHash = ReadLE32(Data, 92);

	// Root ID string (at offset 96, always empty for root = length 0)
	size Pos = 96;
	std::string RootID = ReadString(Data, Pos);

	// Root type name string
	RootTypeName = ReadString(Data, Pos);
	RootDataOffset = Pos;

	// Find EFBEEFBE marker and identify nodes
	_FindBeefMarker();
	_IdentifyNodes();

	return true;
}

// Saves the GFX data back to a binary .gfx file (byte-for-byte from Data).
bool Gfx::Save(const std::string& FilePath)
{
	std::ofstream OutFile(FilePath, std::ios::binary);
	if (!OutFile)
	{
		std::fprintf(stderr, "[Error] [gfx.cpp] Failed to open output file: %s\n", FilePath.c_str());
		return false;
	}

	OutFile.write(reinterpret_cast<const char*>(Data.data()), Data.size());
	OutFile.close();
	return true;
}

// Exports the GFX data to a human-readable XML format for inspection and editing.
bool Gfx::ExportXML(const std::string& FilePath) const
{
	tinyxml2::XMLDocument Doc;
	Doc.InsertEndChild(Doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\""));

	tinyxml2::XMLElement* Root = Doc.NewElement("GfxFile");
	Doc.InsertEndChild(Root);

	auto AddChildText = [&](tinyxml2::XMLElement* Parent, const char* Name, const std::string& Text) {
		tinyxml2::XMLElement* Element = Doc.NewElement(Name);
		Element->SetText(Text.c_str());
		Parent->InsertEndChild(Element);
	};

	// Version
	AddChildText(Root, "Version", std::to_string(Version));

	// Root node info
	{
		char Buf[16];
		std::snprintf(Buf, sizeof(Buf), "%08X", RootTypeHash);
		AddChildText(Root, "RootTypeHash", Buf);
	}
	AddChildText(Root, "RootTypeName", RootTypeName);

	// Complete file data as hex
	AddChildText(Root, "Data", ToHex(Data));

	// Node metadata (informational, not used for import)
	tinyxml2::XMLElement* NodesNode = Doc.NewElement("Nodes");
	Root->InsertEndChild(NodesNode);

	for (size i = 0; i < Nodes.size(); i++)
	{
		const GfxNode& Node = Nodes[i];
		tinyxml2::XMLElement* NodeElem = Doc.NewElement("Node");
		NodeElem->SetAttribute("index", static_cast<int32>(i));
		NodeElem->SetAttribute("offset", static_cast<int32>(Node.Offset));

		char IHashBuf[16], THashBuf[16];
		std::snprintf(IHashBuf, sizeof(IHashBuf), "%08X", Node.InstanceHash);
		std::snprintf(THashBuf, sizeof(THashBuf), "%08X", Node.TypeHash);
		NodeElem->SetAttribute("instanceHash", IHashBuf);
		NodeElem->SetAttribute("typeHash", THashBuf);
		NodeElem->SetAttribute("type", GfxTypeHashToString(Node.TypeHash));
		NodeElem->SetAttribute("id", Node.ID.c_str());
		NodeElem->SetAttribute("name", Node.Name.c_str());

		NodesNode->InsertEndChild(NodeElem);
	}

	// String table (informational)
	tinyxml2::XMLElement* StringsNode = Doc.NewElement("Strings");
	Root->InsertEndChild(StringsNode);

	for (size i = 0; i < Strings.size(); i++)
	{
		tinyxml2::XMLElement* StrElem = Doc.NewElement("S");
		StrElem->SetAttribute("offset", static_cast<int32>(Strings[i].first));
		StrElem->SetText(Strings[i].second.c_str());
		StringsNode->InsertEndChild(StrElem);
	}

	return Doc.SaveFile(FilePath.c_str()) == tinyxml2::XML_SUCCESS;
}

// Imports the GFX data from an XML file previously exported by ExportXML.
bool Gfx::ImportXML(const std::string& FilePath)
{
	tinyxml2::XMLDocument Doc;
	if (Doc.LoadFile(FilePath.c_str()) != tinyxml2::XML_SUCCESS)
	{
		std::fprintf(stderr, "[Error] [gfx.cpp] Failed to parse XML file: %s\n", FilePath.c_str());
		return false;
	}

	tinyxml2::XMLElement* Root = Doc.RootElement();
	if (!Root)
	{
		std::fprintf(stderr, "[Error] [gfx.cpp] No root element in XML: %s\n", FilePath.c_str());
		return false;
	}

	// Parse version
	auto GetChildText = [](tinyxml2::XMLElement* Parent, const char* Name) -> std::string
	{
		tinyxml2::XMLElement* Child = Parent->FirstChildElement(Name);
		if (Child && Child->GetText()) return std::string(Child->GetText());
		return "";
	};

	std::string VersionStr = GetChildText(Root, "Version");
	if (!VersionStr.empty()) Version = static_cast<uint32>(std::stoul(VersionStr));

	// Parse root type info
	std::string RootHashStr = GetChildText(Root, "RootTypeHash");
	if (!RootHashStr.empty()) RootTypeHash = static_cast<uint32>(std::stoul(RootHashStr, nullptr, 16));

	RootTypeName = GetChildText(Root, "RootTypeName");

	// Parse data blob
	std::string DataHex = GetChildText(Root, "Data");
	if (DataHex.empty())
	{
		std::fprintf(stderr, "[Error] [gfx.cpp] No Data element in XML: %s\n", FilePath.c_str());
		return false;
	}
	Data = FromHex(DataHex);

	// Re-parse metadata from the imported data
	_FindBeefMarker();
	_IdentifyNodes();

	// Calculate root data offset
	size Pos = 96;
	ReadString(Data, Pos); // Skip root ID
	ReadString(Data, Pos); // Skip root type name
	RootDataOffset = Pos;

	return true;
}

// Prints a summary of the GFX file to the console.
void Gfx::PrintSummary() const
{
	std::cout << "[+] GFX v" << Version << " | Size: " << Data.size() << " bytes"
			  << " | Root: " << (RootTypeName.empty() ? "(none)" : RootTypeName)
			  << " | Nodes: " << Nodes.size()
			  << " | Strings: " << Strings.size() << "\n\n";

	// Root info
	std::cout << "--- Root Node ---\n";
	std::cout << "  Type Hash: 0x" << std::hex << std::setw(8) << std::setfill('0')
			  << RootTypeHash << std::dec << " (" << GfxTypeHashToString(RootTypeHash) << ")\n";
	std::cout << "  Type Name: " << (RootTypeName.empty() ? "(empty)" : RootTypeName) << "\n";
	std::cout << "  Data Offset: " << RootDataOffset << "\n";

	// Root property field at offset 4
	if (Data.size() >= 8)
	{
		std::cout << "  Property Code: " << ReadLE32(Data, 4) << "\n";
	}
	std::cout << "\n";

	// EFBEEFBE marker
	if (BeefOffset > 0)
	{
		std::cout << "--- Post-Data Section ---\n";
		std::cout << "  EFBEEFBE marker at offset: " << BeefOffset << "\n";
		if (BeefOffset + 8 <= Data.size())
		{
			uint32 PostSize = ReadLE32(Data, BeefOffset + 4);
			std::cout << "  Post-data payload size: " << PostSize << " bytes\n";
		}
		std::cout << "  Scene data: " << BeefOffset << " bytes (offsets 0.." << (BeefOffset - 1) << ")\n";
		std::cout << "  Post data: " << (Data.size() - BeefOffset) << " bytes (offsets " << BeefOffset << ".." << (Data.size() - 1) << ")\n";
		std::cout << "\n";
	}

	// Nodes
	std::cout << "--- Child Nodes (" << Nodes.size() << ") ---\n";
	for (size i = 0; i < Nodes.size(); i++)
	{
		const GfxNode& Node = Nodes[i];
		std::cout << "  [" << std::setw(2) << std::setfill(' ') << i << "] "
				  << std::setw(14) << std::setfill(' ') << std::left << Node.Name << std::right
				  << " | Type: " << std::setw(12) << std::setfill(' ') << std::left << GfxTypeHashToString(Node.TypeHash) << std::right
				  << " | Hash: 0x" << std::hex << std::setw(8) << std::setfill('0') << Node.TypeHash << std::dec
				  << " | IHash: 0x" << std::hex << std::setw(8) << std::setfill('0') << Node.InstanceHash << std::dec
				  << " | ID: " << Node.ID
				  << " | @" << Node.Offset << "\n";
	}
	std::cout << "\n";

	// String table
	std::cout << "--- Strings (" << Strings.size() << ") ---\n";
	for (size i = 0; i < Strings.size(); i++)
	{
		std::string Display = Strings[i].second;
		if (Display.size() > 60)
		{
			Display = Display.substr(0, 57) + "...";
		}
		std::cout << "  @" << std::setw(5) << std::setfill(' ') << Strings[i].first
				  << ": [" << std::setw(3) << Strings[i].second.size() << "] " << Display << "\n";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Scans Data for length-prefixed ASCII strings and identifies consecutive ID+name pairs as nodes.
void Gfx::_IdentifyNodes()
{
	Nodes.clear();
	Strings.clear();

	// First pass: find all length-prefixed ASCII strings
	struct StringEntry
	{
		size Offset;
		uint32 Length;
		std::string Text;
	};
	std::vector<StringEntry> FoundStrings;

	for (size i = 0; i + 4 < Data.size(); i++)
	{
		uint32 Len = ReadLE32(Data, i);
		if (Len >= 1 && Len <= 256 && i + 4 + Len <= Data.size())
		{
			if (IsASCII(Data, i + 4, Len))
			{
				std::string Text(reinterpret_cast<const char*>(Data.data() + i + 4), Len);

				// Validate it looks like a real string (at least 2 chars or known pattern)
				if (Len >= 2 || (Len == 1 && Text[0] >= 'A'))
				{
					FoundStrings.push_back({i, Len, Text});
					Strings.push_back({i, Text});
				}
			}
		}
	}

	// Second pass: find consecutive string pairs that represent ID + name
	// A node is identified when two strings appear consecutively (end of first = start of second)
	// AND there are 8 bytes before the first string (instance_hash + type_hash).
	for (size i = 0; i + 1 < FoundStrings.size(); i++)
	{
		const StringEntry& First = FoundStrings[i];
		const StringEntry& Second = FoundStrings[i + 1];

		size EndOfFirst = First.Offset + 4 + First.Length;

		if (Second.Offset == EndOfFirst && First.Offset >= 8)
		{
			GfxNode Node;
			Node.Offset = First.Offset - 8;
			Node.InstanceHash = ReadLE32(Data, Node.Offset);
			Node.TypeHash = ReadLE32(Data, Node.Offset + 4);
			Node.ID = First.Text;
			Node.Name = Second.Text;
			Node.DataOffset = Second.Offset + 4 + Second.Length;

			// Skip the root node (instance hash = 0 at offset 88, but root is handled separately)
			// Root's first string is at offset 96 with length 0, which won't match our min length 1 filter.
			// But just in case, skip if offset matches root position.
			if (Node.Offset != 88)
			{
				Nodes.push_back(Node);
			}
		}
	}
}

// Finds the EFBEEFBE magic marker in Data and sets BeefOffset.
void Gfx::_FindBeefMarker()
{
	BeefOffset = 0;

	for (size i = 0; i + 4 <= Data.size(); i++)
	{
		if (Data[i] == 0xEF && Data[i + 1] == 0xBE && Data[i + 2] == 0xEF && Data[i + 3] == 0xBE)
		{
			BeefOffset = i;
			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////
