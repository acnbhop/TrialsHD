# XUR File Format (.xur)

**Xbox User Interface Resource** — Compiled XUI binary format used by Trials HD on Xbox 360.

XUR files are the compiled form of XUI (Xbox User Interface) scenes. They define the UI layout
including buttons, images, text, groups, scenes, and other visual elements. All multi-byte
integers are stored in **big-endian** byte order.

---

## File Layout Overview

```
┌─────────────────────────────────────────┐
│ Header (20 bytes)                       │
├─────────────────────────────────────────┤
│ Extra Table (40 bytes, optional)        │  ← Only present when Flags & 1
├─────────────────────────────────────────┤
│ Section Descriptors (N × 12 bytes)      │
├─────────────────────────────────────────┤
│ Section Data (STRN, VECT, QUAT, etc.)   │
└─────────────────────────────────────────┘
```

---

## Header (20 bytes)

| Offset | Size   | Type     | Description                                       |
|--------|--------|----------|---------------------------------------------------|
| 0x00   | 4      | char[4]  | Magic: `XUIB` (ASCII)                             |
| 0x04   | 4      | uint32be | Version (typically `5`)                           |
| 0x08   | 4      | uint32be | Flags (bit 0: extra table present)                |
| 0x0C   | 4      | uint32be | Constant (`0x000C0000`)                           |
| 0x10   | 2      | uint16be | Total file size in bytes                          |
| 0x12   | 2      | uint16be | Section count                                     |

### Flags

| Bit | Meaning                                          |
|-----|--------------------------------------------------|
| 0   | Extra table is present (40 bytes after header)   |

---

## Extra Table (40 bytes, conditional)

Present only when `Flags & 1` is set. Immediately follows the 20-byte header.

Contains **10 × uint32be** values. These relate to timelines, named frames, and other
extended XUI features. Their exact purpose is not fully decoded; they are preserved as-is
for round-tripping.

| Offset         | Size | Type     | Description            |
|----------------|------|----------|------------------------|
| 0x14 + (i × 4)| 4    | uint32be | Extra table entry `i`   |

Total size: 10 × 4 = **40 bytes**.

---

## Section Descriptors (N × 12 bytes)

Immediately follow the header (and extra table, if present). Each descriptor is 12 bytes:

| Offset | Size | Type     | Description                              |
|--------|------|----------|------------------------------------------|
| 0x00   | 4    | char[4]  | Section tag (e.g. `STRN`, `VECT`, etc.)  |
| 0x04   | 4    | uint32be | Absolute byte offset from file start     |
| 0x08   | 4    | uint32be | Section size in bytes                    |

---

## Sections

Section data follows the descriptor table. The known section types are:

### `STRN` — String Table

Contains all UI strings: class names, instance IDs, text content, image paths, etc.

The section is a packed sequence of length-prefixed UTF-16BE strings with **no padding**
between entries:

```
┌────────────────────────────────────────┐
│ uint16be  CharCount                    │  ← Number of UTF-16 code units
│ uint8[CharCount × 2]  UTF-16BE data    │  ← String payload (no null terminator)
├────────────────────────────────────────┤
│ uint16be  CharCount                    │
│ uint8[CharCount × 2]  UTF-16BE data    │
├────────────────────────────────────────┤
│ ...                                    │
└────────────────────────────────────────┘
```

**Encoding details:**
- Each string is preceded by a **uint16be** character count (in UTF-16 code units, not bytes).
- The string payload is `CharCount × 2` bytes of UTF-16 big-endian encoded text.
- Surrogate pairs are used for characters outside the Basic Multilingual Plane (U+10000+).
- There is **no null terminator** stored; the length prefix determines string boundaries.
- Strings are indexed sequentially (string 0 is the first entry, string 1 is the second, etc.).

### `VECT` — Vector Data

Contains packed vector data (positions, sizes, colors, etc.) used by UI elements.
Preserved as raw bytes.

### `QUAT` — Quaternion Data

Contains quaternion rotation data for UI element transforms.
Preserved as raw bytes.

### `CUST` — Custom Data

Contains custom/extended data for UI elements.
Preserved as raw bytes.

### `DATA` — Object Tree

Contains the serialized object tree that defines the UI hierarchy — the relationships
between scenes, groups, buttons, images, text elements, and other XUI objects.
This section references strings from the `STRN` table by index.
Preserved as raw bytes.

---

## Tools

The project includes a command-line tool ([`xur_tool.cpp`](../code/tools/xur_tool.cpp)) for
working with XUR files:

```
xur_tool <input.xur>                  Unpack XUR to XML
xur_tool <input.xml>                  Pack XML back to XUR
xur_tool --print <input.xur>          Dump summary to .txt file
xur_tool --print-console <input.xur>  Print summary to console
xur_tool --export <folder>            Batch export all .xur files to .xml
xur_tool --print-dump <folder>        Batch dump summaries for all .xur files
```

### XML Round-Trip Format

The tool exports XUR files to an editable XML format and can re-import them. The XML
contains:

- **Header metadata**: `Version`, `Flags`, `ExtraTable` (hex-encoded uint32 values).
- **Sections**: Each section stored with its 4-character tag. Non-STRN sections store
  their data as a hex string for lossless round-tripping.
- **Strings**: The parsed string table as individual `<S>` elements with an index
  attribute. Strings containing special characters (`<`, `&`, newlines) use CDATA
  sections.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<XurFile>
  <Version>5</Version>
  <Flags>1</Flags>
  <ExtraTable>00000001 00000002 ...</ExtraTable>
  <Sections>
    <Section tag="STRN" index="0"/>
    <Section tag="VECT" index="1">0A0B0C...</Section>
    <Section tag="DATA" index="2">FF00AB...</Section>
  </Sections>
  <Strings>
    <S i="0">XuiScene</S>
    <S i="1">Button_Start</S>
    <S i="2">Press START to begin</S>
  </Strings>
</XurFile>
```

---

## Implementation

- Parser/serializer: [`xur.hpp`](../code/game/xur.hpp) / [`xur.cpp`](../code/game/xur.cpp)
  in namespace `redlynx::game`
- Class: `Xur` — loads, saves, exports, imports, and prints XUR files
- The `STRN` section is fully parsed into `std::vector<std::string>` (UTF-8) for editing;
  all other sections are stored as raw byte vectors for bit-perfect round-tripping.
