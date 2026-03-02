//
// xus.cpp: Xbox User Interface String (.xus) file parser.
//

// File header
#include "xus.hpp"

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

// Loads a .xus file from disk.
bool Xus::Load(const std::string& FilePath)
{
	std::ifstream File(FilePath, std::ios::binary | std::ios::ate);
	if (!File)
	{
		std::fprintf(stderr, "[Error] [xus.cpp] Failed to open XUS file: %s\n", FilePath.c_str());
		return false;
	}

	size RawSize = File.tellg();
	File.seekg(0, std::ios::beg);
	std::vector<uint8> Buffer(RawSize);
	File.read(reinterpret_cast<char*>(Buffer.data()), RawSize);
	File.close();

	if (RawSize < 12)
	{
		std::fprintf(stderr, "[Error] [xus.cpp] File too small to be a valid XUS: %s\n", FilePath.c_str());
		return false;
	}

	std::span<const uint8> Span(Buffer);

	// Validate magic
	if (Buffer[0] != 'X' || Buffer[1] != 'U' || Buffer[2] != 'I' || Buffer[3] != 'S')
	{
		std::fprintf(stderr, "[Error] [xus.cpp] Invalid XUS magic in: %s\n", FilePath.c_str());
		return false;
	}

	// Parse header
	Version = ReadBE16(Span, 4);
	uint32 FileSize   = ReadBE32(Span, 6);
	uint16 EntryCount = ReadBE16(Span, 10);

	if (FileSize != RawSize)
	{
		std::fprintf(stderr, "[Warning] [xus.cpp] File size mismatch: header says %u, actual %zu: %s\n",
			FileSize, RawSize, FilePath.c_str());
	}

	// Parse string pairs
	Entries.clear();
	Entries.reserve(EntryCount);
	size ReadPos = 12;

	for (uint16 i = 0; i < EntryCount; i++)
	{
		XusEntry Entry;

		// Read value string
		if (ReadPos + 2 > RawSize)
		{
			std::fprintf(stderr, "[Error] [xus.cpp] Unexpected EOF reading value string %u: %s\n", i, FilePath.c_str());
			return false;
		}
		uint16 ValueCharCount = ReadBE16(Span, ReadPos);
		ReadPos += 2;
		size ValueByteCount = static_cast<size>(ValueCharCount) * 2;
		if (ReadPos + ValueByteCount > RawSize)
		{
			std::fprintf(stderr, "[Error] [xus.cpp] Value string %u truncated: %s\n", i, FilePath.c_str());
			return false;
		}
		Entry.Value = _UTF16BEToUTF8(Buffer, ReadPos, ValueCharCount);
		ReadPos += ValueByteCount;

		// Read property path string
		if (ReadPos + 2 > RawSize)
		{
			std::fprintf(stderr, "[Error] [xus.cpp] Unexpected EOF reading path string %u: %s\n", i, FilePath.c_str());
			return false;
		}
		uint16 PathCharCount = ReadBE16(Span, ReadPos);
		ReadPos += 2;
		size PathByteCount = static_cast<size>(PathCharCount) * 2;
		if (ReadPos + PathByteCount > RawSize)
		{
			std::fprintf(stderr, "[Error] [xus.cpp] Path string %u truncated: %s\n", i, FilePath.c_str());
			return false;
		}
		Entry.Path = _UTF16BEToUTF8(Buffer, ReadPos, PathCharCount);
		ReadPos += PathByteCount;

		Entries.push_back(std::move(Entry));
	}

	return true;
}

// Saves the XUS data back to a binary .xus file.
bool Xus::Save(const std::string& FilePath)
{
	std::vector<uint8> Output;
	Output.reserve(4096);

	// Magic
	Output.push_back('X'); Output.push_back('U'); Output.push_back('I'); Output.push_back('S');

	// Version
	WriteBE16(Output, Version);

	// FileSize placeholder (will be patched)
	size FileSizePos = Output.size();
	WriteBE32(Output, 0);

	// Entry count
	WriteBE16(Output, static_cast<uint16>(Entries.size()));

	// Write entry pairs
	for (const auto& Entry : Entries)
	{
		// Value string
		std::vector<uint8> ValueUtf16;
		uint16 ValueCharCount = _UTF8ToUTF16BE(Entry.Value, ValueUtf16);
		WriteBE16(Output, ValueCharCount);
		Output.insert(Output.end(), ValueUtf16.begin(), ValueUtf16.end());

		// Path string
		std::vector<uint8> PathUtf16;
		uint16 PathCharCount = _UTF8ToUTF16BE(Entry.Path, PathUtf16);
		WriteBE16(Output, PathCharCount);
		Output.insert(Output.end(), PathUtf16.begin(), PathUtf16.end());
	}

	// Patch file size
	uint32 TotalSize = static_cast<uint32>(Output.size());
	Output[FileSizePos]     = static_cast<uint8>((TotalSize >> 24) & 0xFF);
	Output[FileSizePos + 1] = static_cast<uint8>((TotalSize >> 16) & 0xFF);
	Output[FileSizePos + 2] = static_cast<uint8>((TotalSize >> 8) & 0xFF);
	Output[FileSizePos + 3] = static_cast<uint8>(TotalSize & 0xFF);

	// Write to disk
	std::ofstream OutFile(FilePath, std::ios::binary);
	if (!OutFile)
	{
		std::fprintf(stderr, "[Error] [xus.cpp] Failed to open output file: %s\n", FilePath.c_str());
		return false;
	}

	OutFile.write(reinterpret_cast<const char*>(Output.data()), Output.size());
	OutFile.close();
	return true;
}

// Exports the XUS data to a human-readable XML format for editing.
bool Xus::ExportXML(const std::string& FilePath, bool ExactLineEndings) const
{
	tinyxml2::XMLDocument Doc;
	Doc.InsertEndChild(Doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\""));

	tinyxml2::XMLElement* Root = Doc.NewElement("XusFile");
	Doc.InsertEndChild(Root);

	// Header metadata
	tinyxml2::XMLElement* VersionNode = Doc.NewElement("Version");
	VersionNode->SetText(std::to_string(Version).c_str());
	Root->InsertEndChild(VersionNode);

	// Entries
	tinyxml2::XMLElement* EntriesNode = Doc.NewElement("Entries");
	Root->InsertEndChild(EntriesNode);

	auto NeedsCData = [](const std::string& Str) -> bool
	{
		return Str.find('\n') != std::string::npos ||
			   Str.find('\r') != std::string::npos ||
			   Str.find('<')  != std::string::npos ||
			   Str.find('&')  != std::string::npos;
	};

	auto SetTextOrCData = [&](tinyxml2::XMLElement* Elem, const std::string& Text)
	{
		if (NeedsCData(Text))
		{
			tinyxml2::XMLText* CData = Doc.NewText(Text.c_str());
			CData->SetCData(true);
			Elem->InsertEndChild(CData);
		}
		else
		{
			Elem->SetText(Text.c_str());
		}
	};

	for (size i = 0; i < Entries.size(); i++)
	{
		tinyxml2::XMLElement* EntryNode = Doc.NewElement("E");
		EntryNode->SetAttribute("i", static_cast<int32>(i));

		std::string Value = ExactLineEndings ? EscapeCR(Entries[i].Value) : Entries[i].Value;
		std::string Path  = ExactLineEndings ? EscapeCR(Entries[i].Path)  : Entries[i].Path;

		tinyxml2::XMLElement* ValueNode = Doc.NewElement("Value");
		SetTextOrCData(ValueNode, Value);
		EntryNode->InsertEndChild(ValueNode);

		tinyxml2::XMLElement* PathNode = Doc.NewElement("Path");
		PathNode->SetText(Path.c_str());
		EntryNode->InsertEndChild(PathNode);

		EntriesNode->InsertEndChild(EntryNode);
	}

	return Doc.SaveFile(FilePath.c_str()) == tinyxml2::XML_SUCCESS;
}

// Imports the XUS data from an XML file previously exported by ExportXML.
bool Xus::ImportXML(const std::string& FilePath, bool ExactLineEndings)
{
	tinyxml2::XMLDocument Doc;
	if (Doc.LoadFile(FilePath.c_str()) != tinyxml2::XML_SUCCESS)
	{
		std::fprintf(stderr, "[Error] [xus.cpp] Failed to parse XML file: %s\n", FilePath.c_str());
		return false;
	}

	tinyxml2::XMLElement* Root = Doc.RootElement();
	if (!Root)
	{
		std::fprintf(stderr, "[Error] [xus.cpp] No root element in XML: %s\n", FilePath.c_str());
		return false;
	}

	// Parse version
	tinyxml2::XMLElement* VersionNode = Root->FirstChildElement("Version");
	if (VersionNode && VersionNode->GetText())
	{
		Version = static_cast<uint16>(std::stoul(VersionNode->GetText()));
	}
	else
	{
		Version = 0x0100; // Default
	}

	// Parse entries
	Entries.clear();
	tinyxml2::XMLElement* EntriesNode = Root->FirstChildElement("Entries");
	if (EntriesNode)
	{
		for (tinyxml2::XMLElement* EntryNode = EntriesNode->FirstChildElement("E");
			 EntryNode; EntryNode = EntryNode->NextSiblingElement("E"))
		{
			XusEntry Entry;

			tinyxml2::XMLElement* ValueNode = EntryNode->FirstChildElement("Value");
			if (ValueNode && ValueNode->GetText())
			{
				Entry.Value = ExactLineEndings ? UnescapeCR(ValueNode->GetText()) : ValueNode->GetText();
			}

			tinyxml2::XMLElement* PathNode = EntryNode->FirstChildElement("Path");
			if (PathNode && PathNode->GetText())
			{
				Entry.Path = ExactLineEndings ? UnescapeCR(PathNode->GetText()) : PathNode->GetText();
			}

			Entries.push_back(std::move(Entry));
		}
	}

	return true;
}

// Prints a summary of the XUS file to the console.
void Xus::PrintSummary() const
{
	std::cout << "[+] XUS v0x" << std::hex << std::setw(4) << std::setfill('0') << Version
			  << std::dec << " | Entries: " << Entries.size() << "\n\n";

	std::cout << "--- Entries (" << Entries.size() << ") ---\n";
	for (size i = 0; i < Entries.size(); i++)
	{
		// Truncate long values for display
		std::string DisplayValue = Entries[i].Value;
		if (DisplayValue.size() > 60)
		{
			DisplayValue = DisplayValue.substr(0, 57) + "...";
		}

		// Replace newlines for display
		for (auto& c : DisplayValue)
		{
			if (c == '\n') c = ' ';
			if (c == '\r') c = ' ';
		}

		std::cout << "  [" << std::setw(3) << std::setfill(' ') << i << "] "
				  << Entries[i].Path << " = \"" << DisplayValue << "\"\n";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
REDLYNX_NAMESPACE_END_ENGINE_ASSET
////////////////////////////////////////////////////////////////////////////////////////////////////
