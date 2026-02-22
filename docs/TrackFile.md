# Track File Format (.trk)

**Trials HD Level Data** — Serialized track format used by RedLynx for Trials HD on Xbox 360.

TRK files contain all physical map geometry, logic triggers, physics constraints (joints),
and environment settings that define a playable level. Because the Xbox 360 uses a PowerPC
architecture, all multi-byte integers and IEEE 754 floats are stored in **big-endian** byte
order and must be byte-swapped when read on x86/x64 (little-endian) hardware.

---

## File Layout Overview

```
┌─────────────────────────────────────────┐
│ Container Header (20 bytes)             │
├─────────────────────────────────────────┤
│ Stripped LZMA Stream                    │  ← Compressed payload (no 8-byte size field)
│                                         │
│  ┌───────────────────────────────────┐  │
│  │ OBJ5 Master Header (6 bytes)      │  │
│  ├───────────────────────────────────┤  │
│  │ 9 × Structure-of-Arrays           │  │  ← Object data (SoA layout)
│  ├───────────────────────────────────┤  │
│  │ FourCC Chunks (TRP1, BGR16, ...)  │  │  ← Logic, lighting, backgrounds
│  ├───────────────────────────────────┤  │
│  │ GLUE Chunk                        │  │  ← Physics constraints (AoS layout)
│  ├───────────────────────────────────┤  │
│  │ Trailing Chunks (PACK, TEND, ...) │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

---

## Container Header (20 bytes)

| Offset | Size | Type   | Description                                                        |
|--------|------|--------|--------------------------------------------------------------------|
| `0x00` | 4    | uint32 | Magic signature: `0xDEADBABE`                                      |
| `0x04` | 4    | uint32 | File version / hash (e.g. `0x7004E8D0`). Exact purpose unknown.    |
| `0x08` | 4    | uint32 | Uncompressed size: exact byte count of the decompressed payload.   |
| `0x0C` | 4    | uint32 | Reserved / padding: usually `0x00000000`.                          |
| `0x10` | 4    | uint32 | Compression flag: usually `0x00000001` (indicates LZMA is active). |

---

## LZMA Compression Stream

Immediately following the header at offset `0x14` is the LZMA compressed payload.
RedLynx uses a **stripped LZMA header** to save space:

- **Bytes 0–4** (file offset `0x14`–`0x18`): Standard 5-byte LZMA properties
  (typically `5D 00 00 02 00`).
- **Missing**: The standard 8-byte uncompressed size field that normally follows the
  properties in a 13-byte LZMA header. RedLynx stores this in the container header
  instead (offset `0x08`).

### Decompression Procedure

1. Read the `Uncompressed Size` from offset `0x08` of the container header.
2. Read the 5-byte LZMA properties from offset `0x14`.
3. Synthesize a standard 13-byte LZMA header in memory:
   `[5 bytes properties] + [8 bytes little-endian uncompressed size]`.
4. Feed the remaining bytes (from offset `0x19` onward) to an LZMA decoder.

---

## Uncompressed Payload — `OBJ5` Map Data

Once decompressed, the raw map data begins. To optimize for the Xbox 360's AltiVec SIMD
processing, RedLynx serializes map objects using a **Structure of Arrays (SoA)** layout
rather than standard Arrays of Structures.

### Master Header (6 bytes)

| Offset | Size | Type   | Description                                            |
|--------|------|--------|--------------------------------------------------------|
| `0x00` | 4    | char[4]| Magic signature: `OBJ5`                                |
| `0x04` | 2    | uint16 | Object count: total number of physics props in the map |

---

## Object Arrays (Structure of Arrays)

Following the object count (at offset `0x06`), the engine lays out **9 consecutive arrays**.
Each array contains one field for every object. To find the byte offset of array `N`, sum
the sizes of all preceding arrays: `Object Count × Bytes Per Item` for each.

| # | Array Name   | Type   | Bytes Per Item | Description                                                         |
|---|--------------|--------|----------------|---------------------------------------------------------------------|
| 0 | Type IDs     | uint8  | 1              | Entity/mesh ID (e.g. `128` = Ramp, `211` = Barrel)                  |
| 1 | Variants     | uint16 | 2              | Object flags, texture variants, or collision masks                  |
| 2 | X Coords     | float  | 4              | X-axis world position                                               |
| 3 | Y Coords     | float  | 4              | Y-axis world position                                               |
| 4 | Z Coords     | float  | 4              | Z-axis depth layer (Trials HD is 2.5D; typically `3.84` for rider)  |
| 5 | Rotation X   | uint8  | 1              | Compressed Euler pitch (0–255 → 0°–360°)                            |
| 6 | Rotation Y   | uint8  | 1              | Compressed Euler yaw                                                |
| 7 | Rotation Z   | uint8  | 1              | Compressed Euler roll                                               |
| 8 | Scale / Extra| uint8  | 1              | Uniform scale factor (`127` = 1.0×)                                 |

To read a single object (e.g. object index `5`), index all 9 arrays at position `[5]`.

```
Array 0 (Type IDs):   [obj0][obj1][obj2][obj3][obj4][obj5]...
Array 1 (Variants):   [obj0    ][obj1    ][obj2    ]...
Array 2 (X Coords):   [obj0        ][obj1        ]...
...and so on for all 9 arrays
```

---

## FourCC Chunks

After the final object array (`Scale / Extra`), the file transitions into a sequential
chunk format identified by **Four-Character Codes** (FourCCs). These define logic,
lighting, and physics links.

| Tag    | Description                              |
|--------|------------------------------------------|
| `TRP1` | Triggers and checkpoints                 |
| `TRG2` | Triggers and checkpoints (variant)       |
| `BGR16`| Backgrounds and skyboxes                 |
| `CMUL` | Color multiplier / lighting parameters   |
| `EXT0` | Extra engine parameters                  |
| `GLUE` | Physics constraints (see below)          |
| `PACK` | End-of-file marker                       |
| `TEND` | Track End marker                         |

---

## Physics Constraints — `GLUE` Chunk

Unlike the SoA object data, physics joints (hinges, sliders, welds) are serialized as an
**Array of Structures (AoS)**. This chunk connects two object indices together so they
behave as a single dynamic obstacle.

### `GLUE` Header (6 bytes)

| Offset | Size | Type   | Description                                      |
|--------|------|--------|--------------------------------------------------|
| `0x00` | 4    | char[4]| Magic signature: `GLUE`                          |
| `0x04` | 2    | uint16 | Joint count: total number of physics constraints |

### Joint Structure (12 bytes per joint)

Following the joint count (offset `0x06` relative to chunk start):

| Offset | Size | Type   | Description                                                      |
|--------|------|--------|------------------------------------------------------------------|
| `0x00` | 2    | uint16 | Object A index: SoA index of the first object                    |
| `0x02` | 2    | uint16 | Object B index: SoA index of the second object                   |
| `0x04` | 4    | uint32 | Parameter A: float or bitmask (hinge limits, break force)        |
| `0x08` | 4    | uint32 | Parameter B: float or bitmask                                    |

**Note:** The value `0x9FCF33F2` appears frequently in Parameters A/B and represents a
null/default engine sentinel value.

---

## Parsing Summary

To mod or parse a `.trk` file, a program must execute these steps:

1. Read the container header; extract uncompressed size and byte-swap to little-endian.
2. Synthesize a 13-byte LZMA header and decompress the stream starting at offset `0x14`.
3. Read the `OBJ5` magic and object count.
4. Dynamically calculate memory boundaries for all 9 object arrays.
5. Search the remainder of the file for `GLUE` and other FourCC tags.
6. Extract the joint count and parse the 12-byte AoS joint definitions.
7. Preserve unrecognized binary regions (`PreGlue` / `PostGlue`) as opaque blobs for
   injection back into the file when repacking.
