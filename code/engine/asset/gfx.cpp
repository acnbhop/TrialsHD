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
#include <cstring>
#include <functional>

// External headers
#include <tinyxml2.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Local Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

// Converts a byte range to a hex string with optional line-wrapping for readability.
static std::string ToHexSegment(const std::vector<uint8>& Data, size Start, size End)
{
	if (Start >= End || Start >= Data.size()) return "";
	if (End > Data.size()) End = Data.size();

	std::string Result;
	Result.reserve((End - Start) * 2);

	for (size i = Start; i < End; i++)
	{
		char Buf[4];
		std::snprintf(Buf, sizeof(Buf), "%02x", Data[i]);
		Result += Buf;
	}

	return Result;
}

// Formats a float for XML output, avoiding unnecessary trailing zeros.
static std::string FormatFloat(f32 Value)
{
	// Handle exact zero
	if (Value == 0.0f) return "0";

	char Buf[64];
	std::snprintf(Buf, sizeof(Buf), "%.6g", Value);
	return Buf;
}

// Formats a hex hash value.
static std::string FormatHash(uint32 Hash)
{
	char Buf[16];
	std::snprintf(Buf, sizeof(Buf), "%08X", Hash);
	return Buf;
}

// Reads 3 consecutive LE floats as a vec3 string "x, y, z".
static std::string ReadVec3String(const std::vector<uint8>& Data, size Offset)
{
	if (Offset + 12 > Data.size()) return "?, ?, ?";
	return FormatFloat(ReadLEFloat(Data, Offset)) + ", " +
	       FormatFloat(ReadLEFloat(Data, Offset + 4)) + ", " +
	       FormatFloat(ReadLEFloat(Data, Offset + 8));
}

// Reads 4 consecutive LE floats as a vec4/quaternion string "x, y, z, w".
static std::string ReadVec4String(const std::vector<uint8>& Data, size Offset)
{
	if (Offset + 16 > Data.size()) return "?, ?, ?, ?";
	return FormatFloat(ReadLEFloat(Data, Offset)) + ", " +
	       FormatFloat(ReadLEFloat(Data, Offset + 4)) + ", " +
	       FormatFloat(ReadLEFloat(Data, Offset + 8)) + ", " +
	       FormatFloat(ReadLEFloat(Data, Offset + 12));
}

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
	_BuildHierarchy();

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

	// =====================================================================================
	// Header section - version and file info
	// =====================================================================================
	tinyxml2::XMLElement* HeaderElem = Doc.NewElement("Header");
	Root->InsertEndChild(HeaderElem);

	AddChildText(HeaderElem, "Version", std::to_string(Version));
	AddChildText(HeaderElem, "FileSize", std::to_string(Data.size()));

	if (Data.size() >= 8)
	{
		AddChildText(HeaderElem, "PropertyCode", std::to_string(ReadLE32(Data, 4)));
	}

	// Root transform (offsets 8-87 contain the root node's transform data).
	// Layout based on binary analysis:
	//   [8..19]   unknown0   - 3 floats (often all zeros)
	//   [20..31]  unknown1   - 3 floats (often all zeros)
	//   [32..43]  position   - 3 floats (world-space position)
	//   [44..55]  unknown2   - 3 floats (often all zeros)
	//   [56..67]  scale      - 3 floats (typically 1, 1, 1)
	//   [68..79]  rotation   - 3 floats + 1 float (often 0, 0, 0 + 1; quaternion-like)
	//   [80..87]  flags      - 2 x uint32/float
	if (Data.size() >= 88)
	{
		tinyxml2::XMLElement* TransformElem = Doc.NewElement("RootTransform");
		HeaderElem->InsertEndChild(TransformElem);

		TransformElem->SetAttribute("position", ReadVec3String(Data, 32).c_str());

		if (Data.size() >= 68)
		{
			TransformElem->SetAttribute("scale", ReadVec3String(Data, 56).c_str());
		}
		if (Data.size() >= 80)
		{
			TransformElem->SetAttribute("rotation", ReadVec4String(Data, 68).c_str());
		}

		// Show the less-understood fields for completeness
		TransformElem->SetAttribute("unknown0", ReadVec3String(Data, 8).c_str());
		TransformElem->SetAttribute("unknown1", ReadVec3String(Data, 20).c_str());
		TransformElem->SetAttribute("unknown2", ReadVec3String(Data, 44).c_str());
		if (Data.size() >= 88)
		{
			std::string FlagsStr = FormatFloat(ReadLEFloat(Data, 80)) + ", " +
			                       FormatFloat(ReadLEFloat(Data, 84));
			TransformElem->SetAttribute("flags", FlagsStr.c_str());
		}
	}

	AddChildText(HeaderElem, "RootTypeHash", FormatHash(RootTypeHash));
	AddChildText(HeaderElem, "RootTypeName", RootTypeName);
	AddChildText(HeaderElem, "RootDataOffset", std::to_string(RootDataOffset));

	if (BeefOffset > 0)
	{
		tinyxml2::XMLElement* MarkerElem = Doc.NewElement("BeefMarker");
		HeaderElem->InsertEndChild(MarkerElem);
		MarkerElem->SetAttribute("offset", static_cast<int32>(BeefOffset));
		MarkerElem->SetAttribute("sceneDataBytes", static_cast<int32>(BeefOffset));
		MarkerElem->SetAttribute("postDataBytes", static_cast<int32>(Data.size() - BeefOffset));
		if (BeefOffset + 8 <= Data.size())
		{
			MarkerElem->SetAttribute("postPayloadSize", static_cast<int32>(ReadLE32(Data, BeefOffset + 4)));
		}
	}

	// =====================================================================================
	// Scene Tree - hierarchical view of nodes (informational)
	// =====================================================================================
	{
		tinyxml2::XMLElement* SceneTree = Doc.NewElement("SceneTree");
		Root->InsertEndChild(SceneTree);

		// Create root scene node
		tinyxml2::XMLElement* RootNodeElem = Doc.NewElement("SceneObject");
		SceneTree->InsertEndChild(RootNodeElem);
		RootNodeElem->SetAttribute("type", GfxTypeHashToString(RootTypeHash));
		RootNodeElem->SetAttribute("typeHash", FormatHash(RootTypeHash).c_str());
		RootNodeElem->SetAttribute("name", RootTypeName.c_str());

		// Determine what data belongs to the root before first child
		size RootDataEnd = BeefOffset > 0 ? BeefOffset : Data.size();
		if (!Nodes.empty())
		{
			// The root's own data ends where the first top-level child's data begins
			// (the area before the child's instance_hash, which is at Node.Offset)
			for (size i = 0; i < Nodes.size(); i++)
			{
				if (Nodes[i].ParentIndex == -1 && Nodes[i].Offset < RootDataEnd)
				{
					RootDataEnd = Nodes[i].Offset;
					break;
				}
			}
		}

		// Show root node data segment
		if (RootDataOffset < RootDataEnd)
		{
			tinyxml2::XMLElement* DataElem = Doc.NewElement("NodeData");
			RootNodeElem->InsertEndChild(DataElem);
			DataElem->SetAttribute("offset", static_cast<int32>(RootDataOffset));
			DataElem->SetAttribute("size", static_cast<int32>(RootDataEnd - RootDataOffset));
			DataElem->SetText(ToHexSegment(Data, RootDataOffset, RootDataEnd).c_str());
		}

		// Recursive lambda to emit child nodes
		std::function<void(tinyxml2::XMLElement*, size)> EmitNode;
		EmitNode = [&](tinyxml2::XMLElement* ParentElem, size NodeIndex)
		{
			const GfxNode& Node = Nodes[NodeIndex];

			// Determine the XML element name from the type
			const char* TypeName = GfxTypeHashToString(Node.TypeHash);
			tinyxml2::XMLElement* NodeElem = Doc.NewElement(TypeName);
			ParentElem->InsertEndChild(NodeElem);

			NodeElem->SetAttribute("name", Node.Name.c_str());
			NodeElem->SetAttribute("id", Node.ID.c_str());
			NodeElem->SetAttribute("typeHash", FormatHash(Node.TypeHash).c_str());
			NodeElem->SetAttribute("instanceHash", FormatHash(Node.InstanceHash).c_str());
			NodeElem->SetAttribute("offset", static_cast<int32>(Node.Offset));

			// Calculate the end of this node's own data (before its first child or next sibling)
			size NodeDataEnd;
			if (!Node.Children.empty())
			{
				// Own data ends where first child's header starts
				NodeDataEnd = Nodes[Node.Children[0]].Offset;
			}
			else
			{
				// No children - data extends to next sibling or parent boundary
				// Find the next node in file order after this one
				NodeDataEnd = BeefOffset > 0 ? BeefOffset : Data.size();

				for (size j = 0; j < Nodes.size(); j++)
				{
					if (Nodes[j].Offset > Node.DataOffset && Nodes[j].Offset < NodeDataEnd)
					{
						// Only count it if it's not a descendant
						bool IsDescendant = false;
						int32 CheckParent = Nodes[j].ParentIndex;
						while (CheckParent != -1)
						{
							if (static_cast<size>(CheckParent) == NodeIndex)
							{
								IsDescendant = true;
								break;
							}
							CheckParent = Nodes[CheckParent].ParentIndex;
						}
						if (!IsDescendant)
						{
							NodeDataEnd = Nodes[j].Offset;
							break;
						}
					}
				}
			}

			// Emit this node's own data as hex
			if (Node.DataOffset < NodeDataEnd)
			{
				tinyxml2::XMLElement* DataElem = Doc.NewElement("NodeData");
				NodeElem->InsertEndChild(DataElem);
				DataElem->SetAttribute("offset", static_cast<int32>(Node.DataOffset));
				DataElem->SetAttribute("size", static_cast<int32>(NodeDataEnd - Node.DataOffset));
				DataElem->SetText(ToHexSegment(Data, Node.DataOffset, NodeDataEnd).c_str());
			}

			// Recurse into children
			for (size ChildIdx : Node.Children)
			{
				EmitNode(NodeElem, ChildIdx);
			}
		};

		// Emit top-level children (those whose ParentIndex == -1)
		for (size i = 0; i < Nodes.size(); i++)
		{
			if (Nodes[i].ParentIndex == -1)
			{
				EmitNode(RootNodeElem, i);
			}
		}
	}

	// =====================================================================================
	// Post-data section (materials, animation controllers, etc.)
	// =====================================================================================
	if (BeefOffset > 0 && BeefOffset < Data.size())
	{
		tinyxml2::XMLElement* PostDataElem = Doc.NewElement("PostData");
		Root->InsertEndChild(PostDataElem);
		PostDataElem->SetAttribute("offset", static_cast<int32>(BeefOffset));
		PostDataElem->SetAttribute("size", static_cast<int32>(Data.size() - BeefOffset));

		// Try to parse post-data strings (e.g. material names)
		if (BeefOffset + 8 <= Data.size())
		{
			uint32 PostPayloadSize = ReadLE32(Data, BeefOffset + 4);
			size PostStart = BeefOffset + 8;

			// Scan for readable strings in post-data
			size ScanPos = PostStart;
			while (ScanPos + 4 < Data.size())
			{
				uint32 Len = ReadLE32(Data, ScanPos);
				if (Len >= 1 && Len <= 256 && ScanPos + 4 + Len <= Data.size() && IsASCII(Data, ScanPos + 4, Len))
				{
					std::string Text(reinterpret_cast<const char*>(Data.data() + ScanPos + 4), Len);
					tinyxml2::XMLElement* StrElem = Doc.NewElement("PostString");
					PostDataElem->InsertEndChild(StrElem);
					StrElem->SetAttribute("offset", static_cast<int32>(ScanPos));
					StrElem->SetAttribute("value", Text.c_str());
					ScanPos += 4 + Len;
					continue;
				}
				ScanPos++;
			}

			(void)PostPayloadSize; // Used above for context
		}

		// Raw post-data hex
		AddChildText(PostDataElem, "RawData", ToHexSegment(Data, BeefOffset, Data.size()));
	}

	// =====================================================================================
	// Flat node list (informational summary)
	// =====================================================================================
	{
		tinyxml2::XMLElement* NodeList = Doc.NewElement("NodeList");
		Root->InsertEndChild(NodeList);
		NodeList->SetAttribute("count", static_cast<int32>(Nodes.size()));

		for (size i = 0; i < Nodes.size(); i++)
		{
			const GfxNode& Node = Nodes[i];
			tinyxml2::XMLElement* NodeElem = Doc.NewElement("Node");
			NodeList->InsertEndChild(NodeElem);

			NodeElem->SetAttribute("index", static_cast<int32>(i));
			NodeElem->SetAttribute("name", Node.Name.c_str());
			NodeElem->SetAttribute("type", GfxTypeHashToString(Node.TypeHash));
			NodeElem->SetAttribute("id", Node.ID.c_str());
			NodeElem->SetAttribute("typeHash", FormatHash(Node.TypeHash).c_str());
			NodeElem->SetAttribute("instanceHash", FormatHash(Node.InstanceHash).c_str());
			NodeElem->SetAttribute("offset", static_cast<int32>(Node.Offset));
			NodeElem->SetAttribute("parent", Node.ParentIndex);

			// Show child names for quick reference
			if (!Node.Children.empty())
			{
				std::string ChildNames;
				for (size j = 0; j < Node.Children.size(); j++)
				{
					if (j > 0) ChildNames += ", ";
					ChildNames += Nodes[Node.Children[j]].Name;
				}
				NodeElem->SetAttribute("children", ChildNames.c_str());
			}
		}
	}

	// =====================================================================================
	// String table (informational)
	// =====================================================================================
	{
		tinyxml2::XMLElement* StringsElem = Doc.NewElement("Strings");
		Root->InsertEndChild(StringsElem);
		StringsElem->SetAttribute("count", static_cast<int32>(Strings.size()));

		for (size i = 0; i < Strings.size(); i++)
		{
			tinyxml2::XMLElement* StrElem = Doc.NewElement("S");
			StrElem->SetAttribute("offset", static_cast<int32>(Strings[i].first));
			StrElem->SetText(Strings[i].second.c_str());
			StringsElem->InsertEndChild(StrElem);
		}
	}

	// =====================================================================================
	// Complete raw data blob (used for byte-accurate round-trip import)
	// =====================================================================================
	{
		tinyxml2::XMLComment* Comment = Doc.NewComment(
			" Raw binary data for byte-accurate round-tripping. "
			"The SceneTree above is informational only. "
			"Editing this hex blob is what changes the actual file. ");
		Root->InsertEndChild(Comment);

		AddChildText(Root, "Data", ToHex(Data));
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

	auto GetChildText = [](tinyxml2::XMLElement* Parent, const char* Name) -> std::string
	{
		tinyxml2::XMLElement* Child = Parent->FirstChildElement(Name);
		if (Child && Child->GetText()) return std::string(Child->GetText());
		return "";
	};

	// Try to read version from new Header element first, then fall back to old layout
	tinyxml2::XMLElement* HeaderElem = Root->FirstChildElement("Header");

	std::string VersionStr;
	std::string RootHashStr;
	std::string RootTypeNameStr;

	if (HeaderElem)
	{
		// New format
		VersionStr = GetChildText(HeaderElem, "Version");
		RootHashStr = GetChildText(HeaderElem, "RootTypeHash");
		RootTypeNameStr = GetChildText(HeaderElem, "RootTypeName");
	}
	else
	{
		// Legacy format (flat elements under root)
		VersionStr = GetChildText(Root, "Version");
		RootHashStr = GetChildText(Root, "RootTypeHash");
		RootTypeNameStr = GetChildText(Root, "RootTypeName");
	}

	if (!VersionStr.empty()) Version = static_cast<uint32>(std::stoul(VersionStr));
	if (!RootHashStr.empty()) RootTypeHash = static_cast<uint32>(std::stoul(RootHashStr, nullptr, 16));
	RootTypeName = RootTypeNameStr;

	// Parse data blob (always at top level)
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
	_BuildHierarchy();

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

	// Root transform
	if (Data.size() >= 68)
	{
		std::cout << "  Position: " << ReadVec3String(Data, 32) << "\n";
		std::cout << "  Scale:    " << ReadVec3String(Data, 56) << "\n";
	}
	if (Data.size() >= 80)
	{
		std::cout << "  Rotation: " << ReadVec4String(Data, 68) << "\n";
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

	// Scene tree (hierarchical)
	std::cout << "--- Scene Tree ---\n";
	std::cout << "  " << (RootTypeName.empty() ? "(root)" : RootTypeName)
			  << " [" << GfxTypeHashToString(RootTypeHash) << "]\n";

	// Print hierarchy using indentation
	std::function<void(size, int)> PrintNodeTree;
	PrintNodeTree = [&](size NodeIndex, int Depth)
	{
		const GfxNode& Node = Nodes[NodeIndex];
		std::string Indent(Depth * 4, ' ');
		std::string Connector = (Depth > 0) ? "|-- " : "";

		std::cout << "  " << Indent << Connector << Node.Name
				  << " [" << GfxTypeHashToString(Node.TypeHash) << "]"
				  << " id=" << Node.ID
				  << " @" << Node.Offset << "\n";

		for (size ChildIdx : Node.Children)
		{
			PrintNodeTree(ChildIdx, Depth + 1);
		}
	};

	for (size i = 0; i < Nodes.size(); i++)
	{
		if (Nodes[i].ParentIndex == -1)
		{
			PrintNodeTree(i, 1);
		}
	}
	std::cout << "\n";

	// Flat node list (for reference)
	std::cout << "--- All Nodes (" << Nodes.size() << ") ---\n";
	for (size i = 0; i < Nodes.size(); i++)
	{
		const GfxNode& Node = Nodes[i];
		std::cout << "  [" << std::setw(2) << std::setfill(' ') << i << "] "
				  << std::setw(14) << std::setfill(' ') << std::left << Node.Name << std::right
				  << " | Type: " << std::setw(12) << std::setfill(' ') << std::left << GfxTypeHashToString(Node.TypeHash) << std::right
				  << " | Hash: 0x" << std::hex << std::setw(8) << std::setfill('0') << Node.TypeHash << std::dec
				  << " | IHash: 0x" << std::hex << std::setw(8) << std::setfill('0') << Node.InstanceHash << std::dec
				  << " | ID: " << Node.ID
				  << " | Parent: " << Node.ParentIndex
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
			Node.ParentIndex = -1;

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

// Builds the parent/child hierarchy for Nodes based on file offset containment.
// Nodes are in file order (depth-first). A node B that appears after node A is a child of A
// if B's offset falls within A's data region. The data region of A extends from A.DataOffset
// to the offset of the next node at the same or higher level (or the beef marker).
void Gfx::_BuildHierarchy()
{
	if (Nodes.empty()) return;

	// Clear any existing hierarchy
	for (auto& Node : Nodes)
	{
		Node.ParentIndex = -1;
		Node.Children.clear();
	}

	// Nodes are already sorted by file offset (they come from a linear scan).
	// We use a stack-based approach: maintain a stack of "open" ancestor nodes.
	// For each new node, pop the stack until we find an ancestor whose data region
	// could contain this node, then make it a child of that ancestor.
	//
	// The data region upper bound for a node is determined by the end of the scene data
	// (BeefOffset or Data.size()) -- we refine this as we discover siblings.

	struct StackEntry
	{
		size NodeIndex;
		size RegionEnd; // Upper bound of this node's data region
	};

	size SceneEnd = BeefOffset > 0 ? BeefOffset : Data.size();

	// Stack represents the current chain of ancestors. The bottom is the implicit root.
	std::vector<StackEntry> Stack;

	for (size i = 0; i < Nodes.size(); i++)
	{
		size NodeStart = Nodes[i].Offset;

		// Pop stack entries whose region ends at or before this node
		while (!Stack.empty() && Stack.back().RegionEnd <= NodeStart)
		{
			Stack.pop_back();
		}

		if (Stack.empty())
		{
			// This node is a top-level child of the root
			Nodes[i].ParentIndex = -1;
		}
		else
		{
			// This node is a child of the top-of-stack
			size ParentIdx = Stack.back().NodeIndex;
			Nodes[i].ParentIndex = static_cast<int32>(ParentIdx);
			Nodes[ParentIdx].Children.push_back(i);
		}

		// Push this node onto the stack. Its region extends to the parent's region end
		// (or SceneEnd if no parent).
		size MyRegionEnd = Stack.empty() ? SceneEnd : Stack.back().RegionEnd;
		Stack.push_back({i, MyRegionEnd});
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