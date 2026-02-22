//
// xur.hpp: Xbox User Interface Resource (.xur) file parser.
//
// XUR files (compiled XUI) are the binary UI scene files used by Trials HD on Xbox 360.
// They contain string tables, vector/quaternion data, custom data, and the object tree
// that defines the UI layout (buttons, images, text, groups, scenes, etc.).
//
// File layout:
//   [20 bytes] Header: magic "XUIB", version, flags, file size, section count
//   [40 bytes] Extra table (only when flags & 1): 10 x uint32be values
//   [N x 12]  Section descriptors: tag(4) + offset(4) + size(4) per section
//   [...]     Section data: STRN, VECT, QUAT, CUST, DATA
//
// The STRN section stores UTF-16BE strings (class names, instance IDs, text content, image
// paths) and is fully parsed for editing. All other sections are preserved as raw bytes to
// ensure 1:1 round-tripping.
//

#pragma once

#include "shared/build_defines.hpp"

#include <vector>
#include <string>
#include <span>

namespace redlynx::game
{

//
// Describes a single section inside the XUR file (STRN, VECT, QUAT, CUST, DATA).
//
struct XurSection
{
    char        Tag[4];     // e.g. "STRN", "VECT", "QUAT", "CUST", "DATA"
    uint32      Offset;     // Byte offset from file start
    uint32      Size;       // Section size in bytes
    std::vector<uint8>  Data;   // Raw section data (for non-STRN sections)
};

//
// XUR file class. Contains all data and functions for reading, saving, exporting and importing
// Xbox User Interface Resource files.
//
class Xur
{
public:
    //
    // Parsed header fields.
    //

    uint32                      Version;        // File version (typically 5)
    uint32                      Flags;          // Bit 0: extra table present
    uint16                      FileSize;       // Total file size stored in header
    uint16                      SectionCount;   // Number of sections

    //
    // Extra table present when Flags & 1. Contains 10 x uint32be values whose purpose
    // relates to timelines, named frames, and other extended XUI features. Preserved
    // as-is for round-tripping.
    //

    std::vector<uint32>         ExtraTable;

    //
    // Section descriptors and raw data. The STRN section data is parsed into the Strings
    // vector below; its raw Data member is left empty and rebuilt on save.
    //

    std::vector<XurSection>     Sections;

    //
    // Parsed string table from the STRN section. Strings are stored as UTF-16BE in the
    // binary but converted to UTF-8 std::string for easy manipulation. Each string is
    // prefixed by a BE16 character count in the binary.
    //

    std::vector<std::string>    Strings;

    // Loads a .xur file from disk.
    bool Load(const std::string& FilePath);

    // Saves the XUR data back to a binary .xur file.
    bool Save(const std::string& FilePath);

    // Exports the XUR data to a human-readable XML format for editing.
    bool ExportXML(const std::string& FilePath) const;

    // Imports the XUR data from an XML file previously exported by ExportXML.
    bool ImportXML(const std::string& FilePath);

    // Prints a summary of the XUR file to the console.
    void PrintSummary() const;

private:
    // Parses the STRN section data into the Strings vector.
    bool _ParseSTRN(const std::vector<uint8>& Data);

    // Rebuilds the STRN section binary data from the Strings vector.
    std::vector<uint8> _BuildSTRN() const;
};

} // namespace redlynx::game
