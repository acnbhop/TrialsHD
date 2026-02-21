# Trials HD Track Format `.trk`

## Overview
The `.trk` file is a level serialization format used by RedLynx for Trials HD (Xbox 360). It contains all physical map geometry, logic triggers, physics constraints (joint), and environment settings.

Because the Xbox 360 uses a PowerPC architecture, the entire file is written in Big-Endian. All 16-bit integers, 32-bit integers and 32-bit IEEE 754 floating-point numbers must be byte-swapped when read on standard x86/64 (Little-Endian) hardware.

# 1. File Container & Compression
The file is wrapped in a 20-byte header, followed immediately by a stripped LZMA compression stream.

## Header Layout (20 bytes)
| Offset | Type | Size | Description |
|--------|------|------|-------------|
`0x00` | `uint32` | 4 bytes | Magic Signature: `0xDEADBABE`
`0x04` | `uint32` | 4 bytes | File Version / Hash: Unknown (e.g., `0x7004E8D0`)
`0x08` | `uint32` | 4 bytes | Uncompressed Size: Exact size of the extracted payload in bytes.
`0x0C` | `uint32` | 4 bytes | Padding / Reserved: Usually `0x00000000`
`0x10` | `uint32` | 4 bytes | Compression Flag: Usually `0x00000001` (indicates LZMA is active).

## The LZMA Stream
Following the header offset `0x14`, is the LZMA compressed payload. To save space, RedLynx utilizes a stripped LZMA header:
* Bytes 0-4 (Offset 0x14 - 0x18): LZMA Properties (usually `5D 00 00 02 00`).
* Catch: Standard LZMA headers are 13 bytes long (5 bytes properties + 8 bytes uncompressed size). RedLynx drops the 8-byte size from the stream entirely.
* Decompression: We must read the `Uncompressed Size` from the proprietary `DEADBABE` header (Offset 0x08), synthesize a fake 13-byte standard LZMA header in memory, then feed the remaining bytes to an LZMA encoder.

# 2. Uncompressed Payload (`OBJ5` Map Data)
Once decompressed, raw map data begins. To optimize for the Xbox 360's AltiVec SIMD processing, RedLynx did not use standard Object arrays (Arrays of Structures). Instead, they serialize map objects using Structure of Array (SoA).

## Master Header
| Offset | Type | Description |
|--------|------|-------------|
| `0x00` | char[4] | Magic Signature: `OBJ5`
| `0x04` | uint16 | Object Count: Total number of physics props/objects in the map.

## Structure of Arrays (Object Data)
Following the `Object Count` (at offset `0x06`), the engine lays out 9 consecutive arrays. To find the byte-offset of an array, you must multiply the `Object Count` by the byte-size of the previous arrays.

| Array Name | Data Type | Bytes Per Item | Description |
|------------|-----------|----------------|-------------|
Type IDs | `uint8` | 1 | The entity/mesh ID (e.g., 128 = Ramp, 211 = Barrel).
Variants | `uint16` | 2 | Object flags, texture variants, or collision masks.
X Coords | `float` | 4 | X-axis world position.
Y Coords | `float` | 4 | Y-axis world position.
Z Coords | `float` | 4 | Z-axis depth layer. Trials HD is 2.5D; Z is mostly static (e.g., `3.84` for the riding lane).
Rotation X | `uint8` | 1 | Compressed Euler Pitch (0-255 mapped to 0-360 degrees).
Rotation Y | `uint8` | 1 | Compressed Euler Yaw.
Rotation Z | `uint8` | 1 | Compressed Euler Roll.
Scale / Extra | `uint8` | 1 | Typically Uniform Scale (where `127` = 1.0x Scale).

To read a single object (e.g., Object Index `5`), you must index all 9 arrays at `[5]`.

# 3. Map Chunks (FourCC Tags)
After the final Object Array (`Scale`), the file transitions into a sequential chunk format identified by Four-Character Codes (FourCCs). These dictate logic, lighting and physics links.

Common chunks include:
* `TRP1` / `TRG2`: Triggers and checkpoints.
* `BGR16`: Backgrounds and skyboxes.
* `CMUL` / `EXT0`: Extra engine parameters.
* `PACK` / `TEND`: End of file markers (`Track End`).

# 4. Physics Constraints (`GLUE` Chunk)
Unlike objects, the physics joints (hinges, sliders, welds) are serialized as an Array of Structures (AoS). This chunk connects two Object IDs together so they behave as a single dynamic obstacle.

## `GLUE` Header
| Offset | Type | Size | Description |
|--------|------|------|-------------|
`0x00` | `char[4]` | 4 bytes | Magic Signature: `GLUE`
`0x04` | `uint16` | 2 bytes | Joint Count: The total number of physics constraints.

## Joint Structure (12 Bytes Per Joint)
Following the `Joint Count` (Offset `0x06` relative to chunk start), the file contains an array of joints.
| Offset | Type | Size | Description |
|--------|------|------|-------------|
`0x00` | `uint16` | 2 bytes | Object A Index: The SoA index of the first object.
`0x02` | `uint16` | 2 bytes | Object B Index: The SoA index of the second object.
`0x04` | `uint32` | 4 bytes | "Parameter A: Float or bitmask (Hinge limits, break force)."
`0x08` | `uint32` | 4 bytes | Parameter B: Float or bitmask.

*Note: `0x9FCF33F2` is frequently used in Parameters A/B and represents a null/default engine value...*

# Summary
To mod or parse a `.trk` file, a program must execute these steps

1. Read the header, extract uncompressed size and byte-swap to little-endian.
2. Synthesize a 13-byte header and decompress the LZMA stream starting at `0x14`.
3. Read the `OBJ5` magic and `Object Count`.
4. Dynamically calculate memory boundaries for all 9 Object Arrays.
5. Search the remainder of the file for the `GLUE` tag.
6. Extract the `Joint Count` and parse the 12-byte AoS joint definitions.
7. Quarantine the remaining binary blobs `PreGlue` and `PostGlue` to inject them back unmodified when repacking the track.
