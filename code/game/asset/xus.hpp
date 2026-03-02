//
// xus.hpp: Xbox User Interface String (.xus) file parser.
//
// XUS files are stand-alone string tables used by Trials HD on Xbox 360 for
// localization. They contain key-value entries that pair a property path (or
// string ID) with a localized text value. The XUI runtime substitutes these
// entries into XUR scene layouts at load time.
//
// File layout:
//   [12 bytes] Header: magic "XUIS", version, file size, entry count
//   [N × 2 strings] Entry pairs: (value, property path) as UTF-16BE
//
// Each string is length-prefixed with a uint16be character count followed by
// CharCount × 2 bytes of UTF-16BE encoded text (no null terminator).
//

#pragma once

#include <vector>
#include <string>

namespace redlynx::game
{

//
// A single entry in the XUS file: a (Value, PropertyPath) pair.
//
struct XusEntry
{
    std::string Value;  // The localized text or numeric value
    std::string Path;   // Property path (e.g. "XuiScene1.Text") or string ID (e.g. "ACHIEVEMENT_ERROR_TEXT")
};

//
// XUS file class. Contains all data and functions for reading, saving, exporting and importing
// Xbox User Interface String files.
//
class Xus
{
public:
    //
    // Parsed header fields.
    //

    uint16                      Version;        // File version (typically 0x0100)

    //
    // Parsed entries. Each entry is a (Value, PropertyPath) pair representing a
    // single UI property override or string table mapping.
    //

    std::vector<XusEntry>       Entries;

    // Loads a .xus file from disk.
    bool Load(const std::string& FilePath);

    // Saves the XUS data back to a binary .xus file.
    bool Save(const std::string& FilePath);

    // Exports the XUS data to a human-readable XML format for editing.
    // When ExactLineEndings is true, \r characters are backslash-escaped so
    // that XML round-tripping preserves them byte-for-byte.
    bool ExportXML(const std::string& FilePath, bool ExactLineEndings = false) const;

    // Imports the XUS data from an XML file previously exported by ExportXML.
    // When ExactLineEndings is true, backslash-escaped \r sequences are restored.
    bool ImportXML(const std::string& FilePath, bool ExactLineEndings = false);

    // Prints a summary of the XUS file to the console.
    void PrintSummary() const;
};

} // namespace redlynx::game
