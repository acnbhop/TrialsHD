//
// xur.cpp: Xbox User Interface Resource (.xur) file parser.
//

// File header
#include "xur.hpp"

// Shared headers
#include "shared/util.hpp"

// Standard headers
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>

// External headers
#include <tinyxml2.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_BEGIN_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Loads a .xur file from disk.
bool Xur::Load(const std::string& FilePath)
{
	std::ifstream File(FilePath, std::ios::binary | std::ios::ate);
	if (!File)
	{
		std::fprintf(stderr, "[Error] [xur.cpp] Failed to open XUR file: %s\n", FilePath.c_str());
		return false;
	}

	size RawSize = File.tellg();
	File.seekg(0, std::ios::beg);
	std::vector<uint8> Buffer(RawSize);
	File.read(reinterpret_cast<char*>(Buffer.data()), RawSize);
	File.close();

	if (RawSize < 20)
	{
		std::fprintf(stderr, "[Error] [xur.cpp] File too small to be a valid XUR: %s\n", FilePath.c_str());
		return false;
	}

	std::span<const uint8> Span(Buffer);

	// Validate magic
	if (Buffer[0] != 'X' || Buffer[1] != 'U' || Buffer[2] != 'I' || Buffer[3] != 'B')
	{
		std::fprintf(stderr, "[Error] [xur.cpp] Invalid XUR magic in: %s\n", FilePath.c_str());
		return false;
	}

	// Parse header
	Version      = ReadBE32(Span, 4);
	Flags        = ReadBE32(Span, 8);
	FileSize     = ReadBE16(Span, 16);
	SectionCount = ReadBE16(Span, 18);

	size ReadPos = 20;

	// Parse extra table if flags & 1
	ExtraTable.clear();
	if (Flags & 1)
	{
		if (ReadPos + 40 > RawSize)
		{
			std::fprintf(stderr, "[Error] [xur.cpp] File too small for extra table: %s\n", FilePath.c_str());
			return false;
		}

		for (int32 i = 0; i < 10; i++)
		{
			ExtraTable.push_back(ReadBE32(Span, ReadPos));
			ReadPos += 4;
		}
	}

	// Parse section descriptors
	Sections.clear();
	Sections.resize(SectionCount);

	for (uint16 i = 0; i < SectionCount; i++)
	{
		if (ReadPos + 12 > RawSize)
		{
			std::fprintf(stderr, "[Error] [xur.cpp] File too small for section descriptor %u: %s\n", i, FilePath.c_str());
			return false;
		}

		Sections[i].Tag[0] = static_cast<char>(Buffer[ReadPos]);
		Sections[i].Tag[1] = static_cast<char>(Buffer[ReadPos + 1]);
		Sections[i].Tag[2] = static_cast<char>(Buffer[ReadPos + 2]);
		Sections[i].Tag[3] = static_cast<char>(Buffer[ReadPos + 3]);
		Sections[i].Offset = ReadBE32(Span, ReadPos + 4);
		Sections[i].Size   = ReadBE32(Span, ReadPos + 8);
		ReadPos += 12;
	}

	// Load section data
	Strings.clear();
	for (uint16 i = 0; i < SectionCount; i++)
	{
		uint32 Off = Sections[i].Offset;
		uint32 Sz  = Sections[i].Size;

		if (Off + Sz > RawSize)
		{
			std::fprintf(stderr, "[Error] [xur.cpp] Section '%c%c%c%c' extends beyond file: %s\n",
				Sections[i].Tag[0], Sections[i].Tag[1], Sections[i].Tag[2], Sections[i].Tag[3],
				FilePath.c_str());
			return false;
		}

		std::string TagStr(Sections[i].Tag, 4);

		if (TagStr == "STRN")
		{
			// Parse string table
			std::vector<uint8> StrnData(Buffer.begin() + Off, Buffer.begin() + Off + Sz);
			if (!_ParseSTRN(StrnData))
			{
				std::fprintf(stderr, "[Error] [xur.cpp] Failed to parse STRN section: %s\n", FilePath.c_str());
				return false;
			}
			// STRN raw data is NOT stored — it will be rebuilt from Strings on save.
		}
		else
		{
			// Store raw section data for round-tripping
			Sections[i].Data.assign(Buffer.begin() + Off, Buffer.begin() + Off + Sz);
		}
	}

	return true;
}

// Saves the XUR data back to a binary .xur file.
bool Xur::Save(const std::string& FilePath)
{
	// Rebuild STRN from current Strings vector
	std::vector<uint8> StrnData = _BuildSTRN();

	// Calculate section data for layout
	// Header: 20 bytes
	// Extra table: 40 bytes if Flags & 1
	// Section descriptors: SectionCount * 12
	size HeaderSize = 20;
	if (Flags & 1) HeaderSize += 40;
	HeaderSize += static_cast<size>(Sections.size()) * 12;

	// Build the output buffer
	std::vector<uint8> Output;

	// Reserve a reasonable amount
	Output.reserve(HeaderSize + StrnData.size() + 4096);

	// Magic
	Output.push_back('X'); Output.push_back('U'); Output.push_back('I'); Output.push_back('B');

	// Version
	WriteBE32(Output, Version);

	// Flags
	WriteBE32(Output, Flags);

	// Bytes 12-15: preserved constant (0x000C0000)
	Output.push_back(0x00); Output.push_back(0x0C);
	Output.push_back(0x00); Output.push_back(0x00);

	// FileSize and SectionCount — placeholder, will be patched after layout
	size FileSizePos = Output.size();
	WriteBE16(Output, 0); // FileSize placeholder
	WriteBE16(Output, static_cast<uint16>(Sections.size()));

	// Extra table
	if (Flags & 1)
	{
		for (size i = 0; i < 10; i++)
		{
			if (i < ExtraTable.size())
				WriteBE32(Output, ExtraTable[i]);
			else
				WriteBE32(Output, 0);
		}
	}

	// Section descriptor placeholders — we'll patch offsets after laying out section data
	size DescriptorStart = Output.size();
	for (size i = 0; i < Sections.size(); i++)
	{
		Output.push_back(static_cast<uint8>(Sections[i].Tag[0]));
		Output.push_back(static_cast<uint8>(Sections[i].Tag[1]));
		Output.push_back(static_cast<uint8>(Sections[i].Tag[2]));
		Output.push_back(static_cast<uint8>(Sections[i].Tag[3]));
		WriteBE32(Output, 0); // Offset placeholder
		WriteBE32(Output, 0); // Size placeholder
	}

	// Now append section data in order and patch descriptors
	for (size i = 0; i < Sections.size(); i++)
	{
		std::string TagStr(Sections[i].Tag, 4);

		uint32 SectionOffset = static_cast<uint32>(Output.size());
		uint32 SectionSize = 0;

		if (TagStr == "STRN")
		{
			Output.insert(Output.end(), StrnData.begin(), StrnData.end());
			SectionSize = static_cast<uint32>(StrnData.size());
		}
		else
		{
			Output.insert(Output.end(), Sections[i].Data.begin(), Sections[i].Data.end());
			SectionSize = static_cast<uint32>(Sections[i].Data.size());
		}

		// Patch offset and size in the descriptor
		size DescOff = DescriptorStart + (i * 12) + 4; // +4 to skip tag
		Output[DescOff]     = static_cast<uint8>((SectionOffset >> 24) & 0xFF);
		Output[DescOff + 1] = static_cast<uint8>((SectionOffset >> 16) & 0xFF);
		Output[DescOff + 2] = static_cast<uint8>((SectionOffset >> 8) & 0xFF);
		Output[DescOff + 3] = static_cast<uint8>(SectionOffset & 0xFF);
		Output[DescOff + 4] = static_cast<uint8>((SectionSize >> 24) & 0xFF);
		Output[DescOff + 5] = static_cast<uint8>((SectionSize >> 16) & 0xFF);
		Output[DescOff + 6] = static_cast<uint8>((SectionSize >> 8) & 0xFF);
		Output[DescOff + 7] = static_cast<uint8>(SectionSize & 0xFF);
	}

	// Patch total file size
	uint16 TotalSize = static_cast<uint16>(Output.size());
	Output[FileSizePos]     = static_cast<uint8>((TotalSize >> 8) & 0xFF);
	Output[FileSizePos + 1] = static_cast<uint8>(TotalSize & 0xFF);

	// Write to disk
	std::ofstream OutFile(FilePath, std::ios::binary);
	if (!OutFile)
	{
		std::fprintf(stderr, "[Error] [xur.cpp] Failed to open output file: %s\n", FilePath.c_str());
		return false;
	}

	OutFile.write(reinterpret_cast<const char*>(Output.data()), Output.size());
	OutFile.close();
	return true;
}

// Exports the XUR data to a human-readable XML format for editing.
bool Xur::ExportXML(const std::string& FilePath, bool ExactLineEndings) const
{
	tinyxml2::XMLDocument Doc;
	Doc.InsertEndChild(Doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\""));

	tinyxml2::XMLElement* Root = Doc.NewElement("XurFile");
	Doc.InsertEndChild(Root);

	auto AddChildText = [&](tinyxml2::XMLElement* Parent, const char* Name, const std::string& Text) {
		tinyxml2::XMLElement* Element = Doc.NewElement(Name);
		Element->SetText(Text.c_str());
		Parent->InsertEndChild(Element);
	};

	// Header metadata
	AddChildText(Root, "Version", std::to_string(Version));
	AddChildText(Root, "Flags", std::to_string(Flags));

	// Extra table
	if (!ExtraTable.empty())
	{
		std::string ExtraHex;
		for (size i = 0; i < ExtraTable.size(); i++)
		{
			if (i > 0) ExtraHex += ' ';
			char Buf[16];
			std::snprintf(Buf, sizeof(Buf), "%08X", ExtraTable[i]);
			ExtraHex += Buf;
		}
		AddChildText(Root, "ExtraTable", ExtraHex);
	}

	// Section descriptors (tag and raw data for non-STRN sections)
	tinyxml2::XMLElement* SectionsNode = Doc.NewElement("Sections");
	Root->InsertEndChild(SectionsNode);

	for (size i = 0; i < Sections.size(); i++)
	{
		tinyxml2::XMLElement* SecNode = Doc.NewElement("Section");
		std::string TagStr(Sections[i].Tag, 4);
		SecNode->SetAttribute("tag", TagStr.c_str());
		SecNode->SetAttribute("index", static_cast<int32>(i));

		if (TagStr != "STRN")
		{
			// Store raw data as hex
			SecNode->SetText(ToHex(Sections[i].Data).c_str());
		}

		SectionsNode->InsertEndChild(SecNode);
	}

	// String table — the editable part
	tinyxml2::XMLElement* StringsNode = Doc.NewElement("Strings");
	Root->InsertEndChild(StringsNode);

	for (size i = 0; i < Strings.size(); i++)
	{
		tinyxml2::XMLElement* StrNode = Doc.NewElement("S");
		StrNode->SetAttribute("i", static_cast<int32>(i));

		std::string Str = ExactLineEndings ? EscapeCR(Strings[i]) : Strings[i];

		// Use CDATA for strings that contain special characters or newlines
		if (Str.find('\n') != std::string::npos ||
			Str.find('<') != std::string::npos ||
			Str.find('&') != std::string::npos)
		{
			tinyxml2::XMLText* CData = Doc.NewText(Str.c_str());
			CData->SetCData(true);
			StrNode->InsertEndChild(CData);
		}
		else
		{
			StrNode->SetText(Str.c_str());
		}

		StringsNode->InsertEndChild(StrNode);
	}

	return Doc.SaveFile(FilePath.c_str()) == tinyxml2::XML_SUCCESS;
}

// Imports the XUR data from an XML file previously exported by ExportXML.
bool Xur::ImportXML(const std::string& FilePath, bool ExactLineEndings)
{
	tinyxml2::XMLDocument Doc;
	if (Doc.LoadFile(FilePath.c_str()) != tinyxml2::XML_SUCCESS)
	{
		std::fprintf(stderr, "[Error] [xur.cpp] Failed to parse XML file: %s\n", FilePath.c_str());
		return false;
	}

	tinyxml2::XMLElement* Root = Doc.RootElement();
	if (!Root)
	{
		std::fprintf(stderr, "[Error] [xur.cpp] No root element in XML: %s\n", FilePath.c_str());
		return false;
	}

	// Parse header values
	auto GetChildText = [](tinyxml2::XMLElement* Parent, const char* Name) -> std::string
	{
		tinyxml2::XMLElement* Child = Parent->FirstChildElement(Name);
		if (Child && Child->GetText()) return std::string(Child->GetText());
		return "";
	};

	std::string VersionStr = GetChildText(Root, "Version");
	std::string FlagsStr   = GetChildText(Root, "Flags");
	if (!VersionStr.empty()) Version = std::stoul(VersionStr);
	if (!FlagsStr.empty())   Flags   = std::stoul(FlagsStr);

	// Parse extra table
	ExtraTable.clear();
	std::string ExtraStr = GetChildText(Root, "ExtraTable");
	if (!ExtraStr.empty())
	{
		// Space-separated hex uint32 values
		std::istringstream ISS(ExtraStr);
		std::string Token;
		while (ISS >> Token)
		{
			ExtraTable.push_back(static_cast<uint32>(std::stoul(Token, nullptr, 16)));
		}
	}

	// Parse sections
	Sections.clear();
	tinyxml2::XMLElement* SectionsNode = Root->FirstChildElement("Sections");
	if (SectionsNode)
	{
		for (tinyxml2::XMLElement* SecNode = SectionsNode->FirstChildElement("Section");
			 SecNode; SecNode = SecNode->NextSiblingElement("Section"))
		{
			XurSection Sec = {};
			const char* TagAttr = SecNode->Attribute("tag");
			if (TagAttr && std::strlen(TagAttr) == 4)
			{
				Sec.Tag[0] = TagAttr[0];
				Sec.Tag[1] = TagAttr[1];
				Sec.Tag[2] = TagAttr[2];
				Sec.Tag[3] = TagAttr[3];
			}

			std::string TagStr(Sec.Tag, 4);
			if (TagStr != "STRN")
			{
				const char* HexText = SecNode->GetText();
				if (HexText)
				{
					Sec.Data = FromHex(std::string(HexText));
				}
			}

			Sections.push_back(Sec);
		}
	}

	SectionCount = static_cast<uint16>(Sections.size());

	// Parse strings
	Strings.clear();
	tinyxml2::XMLElement* StringsNode = Root->FirstChildElement("Strings");
	if (StringsNode)
	{
		for (tinyxml2::XMLElement* StrNode = StringsNode->FirstChildElement("S");
			 StrNode; StrNode = StrNode->NextSiblingElement("S"))
		{
			const char* Text = StrNode->GetText();
			std::string Str = Text ? std::string(Text) : "";
			Strings.push_back(ExactLineEndings ? UnescapeCR(Str) : Str);
		}
	}

	return true;
}

// Prints a summary of the XUR file to the console.
void Xur::PrintSummary() const
{
	std::cout << "[+] XUR v" << Version << " | Flags: " << Flags
			  << " | Sections: " << Sections.size()
			  << " | Strings: " << Strings.size() << "\n\n";

	if (!ExtraTable.empty())
	{
		std::cout << "--- Extra Table ---\n";
		for (size i = 0; i < ExtraTable.size(); i++)
		{
			std::cout << "  [" << i << "] 0x" << std::hex << std::setw(8) << std::setfill('0')
					  << ExtraTable[i] << std::dec << " (" << ExtraTable[i] << ")\n";
		}
		std::cout << "\n";
	}

	std::cout << "--- Sections ---\n";
	for (size i = 0; i < Sections.size(); i++)
	{
		std::string TagStr(Sections[i].Tag, 4);
		std::cout << "  [" << i << "] " << TagStr
				  << " | Offset: 0x" << std::hex << Sections[i].Offset
				  << " | Size: 0x" << Sections[i].Size << std::dec
				  << " (" << Sections[i].Size << " bytes)\n";
	}
	std::cout << "\n";

	std::cout << "--- String Table (" << Strings.size() << " entries) ---\n";
	for (size i = 0; i < Strings.size(); i++)
	{
		// Truncate long strings in the summary
		std::string Display = Strings[i];
		if (Display.size() > 80)
		{
			Display = Display.substr(0, 77) + "...";
		}

		// Replace newlines for display
		for (auto& c : Display)
		{
			if (c == '\n') c = ' ';
			if (c == '\r') c = ' ';
		}

		std::cout << "  [" << std::setw(3) << std::setfill(' ') << i << "] " << Display << "\n";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Parses the STRN section data into the Strings vector.
bool Xur::_ParseSTRN(const std::vector<uint8>& Data)
{
	Strings.clear();
	size Pos = 0;

	while (Pos + 2 <= Data.size())
	{
		uint16 CharCount = (static_cast<uint16>(Data[Pos]) << 8) | Data[Pos + 1];
		Pos += 2;

		size ByteCount = static_cast<size>(CharCount) * 2;
		if (Pos + ByteCount > Data.size())
		{
			std::fprintf(stderr, "[Warning] [xur.cpp] STRN string at offset %zu truncated (need %zu chars, only %zu bytes left)\n",
				Pos - 2, static_cast<size>(CharCount), Data.size() - Pos);
			break;
		}

		Strings.push_back(_UTF16BEToUTF8(Data, Pos, CharCount));
		Pos += ByteCount;
	}

	return true;
}

// Rebuilds the STRN section binary data from the Strings vector.
std::vector<uint8> Xur::_BuildSTRN() const
{
	std::vector<uint8> Data;

	for (const auto& Str : Strings)
	{
		// Convert UTF-8 string to UTF-16BE and prepend character count
		std::vector<uint8> Utf16Bytes;
		uint16 CharCount = _UTF8ToUTF16BE(Str, Utf16Bytes);

		// Write BE16 character count
		Data.push_back(static_cast<uint8>((CharCount >> 8) & 0xFF));
		Data.push_back(static_cast<uint8>(CharCount & 0xFF));

		// Write UTF-16BE data
		Data.insert(Data.end(), Utf16Bytes.begin(), Utf16Bytes.end());
	}

	return Data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////
