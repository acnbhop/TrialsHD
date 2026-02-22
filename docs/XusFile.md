# XUS File Format (.xus)

**Xbox User Interface String** — Compiled XUI string table used by Trials HD on Xbox 360.

XUS files are stand-alone string tables that provide localized text and property overrides
for XUI scenes. They are the companion to [XUR files](XurFile.md): while XUR files define
the overall UI layout and structure, XUS files supply the locale-specific content that gets
substituted into those layouts at runtime.

Each XUS file contains a flat list of key-value **entries**. An entry pairs a **value**
string (the localized text or numeric constant) with a **property path** string (identifying
which UI element and property the value applies to). All multi-byte integers and string
data are stored in **big-endian** byte order, consistent with the Xbox 360's PowerPC
architecture.

---

## File Layout Overview

```
┌───────────────────────────────────────────┐
│ Header (12 bytes)                         │
├───────────────────────────────────────────┤
│ Entry 0: Value String (UTF-16BE)          │
│ Entry 0: Property Path String (UTF-16BE)  │
├───────────────────────────────────────────┤
│ Entry 1: Value String (UTF-16BE)          │
│ Entry 1: Property Path String (UTF-16BE)  │
├───────────────────────────────────────────┤
│ ...                                       │
├───────────────────────────────────────────┤
│ Entry N-1: Value String (UTF-16BE)        │
│ Entry N-1: Property Path String (UTF-16BE)│
└───────────────────────────────────────────┘
```

---

## Header (12 bytes)

| Offset | Size | Type     | Description                                          |
|--------|------|----------|------------------------------------------------------|
| `0x00` | 4    | char[4]  | Magic signature: `XUIS` (ASCII)                      |
| `0x04` | 2    | uint16be | Version (typically `0x0100`)                         |
| `0x06` | 4    | uint32be | Total file size in bytes                             |
| `0x0A` | 2    | uint16be | Entry count: number of key-value pairs               |

---

## String Entries

Immediately following the 12-byte header is a sequence of **EntryCount × 2** length-prefixed
UTF-16BE strings. Entries are stored as consecutive pairs:

```
For each entry i (0 ≤ i < EntryCount):
    String 2i     = Value string    (the localized text or property value)
    String 2i + 1 = Property path   (the target UI element + property name)
```

Each individual string uses the same encoding as the [XUR STRN section](XurFile.md):

| Field                   | Size             | Type     | Description                            |
|-------------------------|------------------|----------|----------------------------------------|
| Character count         | 2                | uint16be | Number of UTF-16 code units            |
| String payload          | CharCount × 2    | UTF-16BE | String data (no null terminator)       |

There is **no padding** between strings.

---

## Entry Structure

Each entry is a (Value, PropertyPath) pair describing a single UI property override:

| String | Role          | Examples                                                          |
|--------|---------------|-------------------------------------------------------------------|
| Value  | Content data  | `"35.000000"`, `"Mitwirkende:"`, `"\r\n    "`, `"Air condenser"`  |
| Path   | Target path   | `"XuiScene1.AnimGroup.Text"`, `"ACHIEVEMENT_ERROR_TEXT"`          |

### Property Path Variants

Property paths come in two styles depending on the XUS file's role:

- **Scene overrides** (locale `.xus` files in `de-de/`, `fr-fr/`, etc.):
  Dot-separated XUI object paths targeting specific properties, e.g.
  `XuiScene1.ESRBNoticeText.PointSize` or `XuiScene1.AnimGroup.CreditsGroup.Title1.Text`.

- **Global string tables** (`strings.xus`, `pack0.xus` in the root Xui folder):
  Simple string IDs used as lookup keys, e.g. `ACHIEVEMENT_ERROR_TEXT`,
  `AIR_CONDENSER`, `ALREADY_HAVE_DOWNLOADED_TRACK_TEXT`.

---

## File Variants

XUS files appear in two contexts within the Trials HD data:

| Location               | Purpose                     | Path Style         | Example                          |
|------------------------|-----------------------------|--------------------|----------------------------------|
| `data/Xui/*.xus`      | Global string tables        | String ID keys     | `strings.xus`, `pack0.xus`        |
| `data/Xui/<locale>/*.xus` | Localized scene overrides | XUI property paths | `de-de/in_game.xus`, `ja-jp/credits.xus` |

The root `.xur` files contain the default (English) UI scenes. Each locale folder contains
`.xus` files that override specific strings and properties for that language. Locale `.xus`
files share the same base name as their corresponding `.xur` file (e.g. `in_game.xur` →
`de-de/in_game.xus`).

---

## Example: Decoded File

Given `de-de/startup.xus` (360 bytes, 4 entries):

```
Header:
  Magic:       XUIS
  Version:     0x0100
  FileSize:    360
  EntryCount:  4

Entry 0:
  Value: "18.000000"
  Path:  "XuiScene1.ESRBNoticeText.PointSize"

Entry 1:
  Value: "Keine ESRB-Einstufung der Onlineinteraktionen "
  Path:  "XuiScene1.ESRBNoticeText.Text"

Entry 2:
  Value: "-1.000000"
  Path:  "XuiScene1.PointSize"

Entry 3:
  Value: "\r\n    "
  Path:  "XuiScene1.Text"
```

---

## Tools

The project includes a command-line tool ([`xus_tool.cpp`](../code/tools/xus_tool.cpp)) for
working with XUS files:

```
xus_tool <input.xus>                  Unpack XUS to XML
xus_tool <input.xml>                  Pack XML back to XUS
xus_tool --print <input.xus>          Dump summary to .txt file
xus_tool --print-console <input.xus>  Print summary to console
xus_tool --export <folder>            Batch export all .xus files to .xml
xus_tool --print-dump <folder>        Batch dump summaries for all .xus files
```

### XML Round-Trip Format

The tool exports XUS files to an editable XML format and can re-import them:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<XusFile>
  <Version>256</Version>
  <Entries>
    <E i="0">
      <Value>18.000000</Value>
      <Path>XuiScene1.ESRBNoticeText.PointSize</Path>
    </E>
    <E i="1">
      <Value>Keine ESRB-Einstufung der Onlineinteraktionen </Value>
      <Path>XuiScene1.ESRBNoticeText.Text</Path>
    </E>
  </Entries>
</XusFile>
```
