# TEX File Format (.tex)

The `.tex` file format is the texture container used by Trials HD's RedLynx engine on Xbox 360.
Each `.tex` file stores a single texture surface with full mipmap chains and optional sprite-sheet
frame metadata. The pixel data is block-compressed (DXT/BCn) or uncompressed, stored in **Xbox 360
GPU tiled layout** with **big-endian 16-bit word order**.

All header fields are **little-endian** unless otherwise noted. Pixel data follows Xbox 360 conventions.

---

## Overview

There are two known versions of the format, distinguished by their 3-byte magic signature:

| Magic | Version | Header Size                    | Description                        |
|-------|---------|--------------------------------|------------------------------------|
| `T4X` | 4       | `10 + FrameCount * 18 + 2`    | Standard format (2563 files found) |
| `T3X` | 3       | `10 + FrameCount * 18 + 1`    | Legacy format (3 files found)      |

The vast majority of textures use `T4X`. The only known `T3X` files are `noise.tex` (uncompressed
ARGB8 noise textures used for particle effects). T3X is identical to T4X except the trailing
content byte at the end of the header is absent (1 byte shorter).

---

## High-Level Layout

```
[3 bytes]   Magic signature ("T4X" or "T3X")
[2 bytes]   Total width (LE uint16, in pixels)
[2 bytes]   Total height (LE uint16, in pixels)
[1 byte]    Pixel format
[1 byte]    Mipmap level count
[1 byte]    Frame count (1 for static textures, >1 for sprite sheets)
[N bytes]   Frame descriptors (N = FrameCount × 18 bytes)
[1-2 bytes] Trailing bytes
[M bytes]   Pixel data (tiled, mip chain for all levels)
```

---

## Header Fields

### Base Header (10 bytes)

| Offset | Size   | Type   | Description                                       |
|--------|--------|--------|---------------------------------------------------|
| 0      | 3      | char[] | Magic: `T4X` (0x54 0x34 0x58) or `T3X` (0x54 0x33 0x58) |
| 3      | 2      | uint16 | Total texture width in pixels (LE)                |
| 5      | 2      | uint16 | Total texture height in pixels (LE)               |
| 7      | 1      | uint8  | Pixel format (see format table below)             |
| 8      | 1      | uint8  | Number of mipmap levels (including base level)    |
| 9      | 1      | uint8  | Frame count (1 = static, >1 = animated/atlas)     |

**Note:** The width and height fields store the *total atlas dimensions* for sprite sheets.
For static textures (frame count = 1), these are the actual texture dimensions. For animated
textures, divide by the grid layout to get individual frame dimensions.

### Pixel Format

| Value  | Format         | Block Size  | Bytes/Block | Bits/Pixel | Description                |
|--------|----------------|-------------|-------------|------------|----------------------------|
| `0x02` | A8R8G8B8       | 1×1 pixel   | 4           | 32         | Uncompressed 32-bit ARGB   |
| `0x0C` | DXT1 (BC1)     | 4×4 pixels  | 8           | 4          | Block-compressed, 1-bit alpha |
| `0x0E` | DXT5 (BC3)     | 4×4 pixels  | 16          | 8          | Block-compressed, interpolated alpha |

These values correspond to the Xbox 360 `GPUTEXTUREFORMAT` enumeration:
- `0x02` = `GPUTEXTUREFORMAT_8_8_8_8`
- `0x0C` = `GPUTEXTUREFORMAT_DXT1`
- `0x0E` = `GPUTEXTUREFORMAT_DXT4_5`

**Distribution across the game's data:** DXT5 accounts for ~98% of textures (2522 files), DXT1 for
~1.2% (32 files, mostly lightmasks), and A8R8G8B8 for ~0.5% (12 files, mostly noise/particle textures).

### Mipmap Count

The mipmap count includes the base level. Mip levels are generated down to 4×4 pixels (the
minimum DXT block size). For DXT textures:

| Base Size   | Mip Count | Levels (pixels)                                   |
|-------------|-----------|---------------------------------------------------|
| 4×4         | 1         | 4                                                 |
| 16×16       | 3         | 16, 8, 4                                          |
| 32×32       | 4         | 32, 16, 8, 4                                      |
| 64×64       | 5         | 64, 32, 16, 8, 4                                  |
| 128×128     | 6         | 128, 64, 32, 16, 8, 4                             |
| 256×256     | 7         | 256, 128, 64, 32, 16, 8, 4                        |
| 512×512     | 8         | 512, 256, 128, 64, 32, 16, 8, 4                   |
| 1024×1024   | 9         | 1024, 512, 256, 128, 64, 32, 16, 8, 4             |

For non-square textures, the mip count is determined by the *smaller* dimension reaching 4.

For uncompressed `A8R8G8B8` textures with `T3X` magic, some files have `mips=1` and `flags=0`,
storing only the base level with no mipmap chain.

---

## Frame Descriptors

Immediately after the 10-byte base header, there are `FrameCount` frame descriptor blocks, each
18 bytes:

### Frame Descriptor (18 bytes each)

| Offset | Size | Type   | Description                                        |
|--------|------|--------|----------------------------------------------------|
| 0      | 1    | uint8  | Reserved (always `0x00`)                           |
| 1      | 2    | uint16 | Frame X offset in atlas, in pixels (LE)            |
| 3      | 2    | uint16 | Frame Y offset in atlas, in pixels (LE)            |
| 5      | 4    | —      | Reserved (always `0x00000000`)                     |
| 9      | 2    | uint16 | Frame width (LE) — repeated 2×                    |
| 11     | 2    | uint16 | Frame height (LE) — repeated 2×                   |
| 13     | 2    | uint16 | Frame width (LE) — duplicate                      |
| 15     | 2    | uint16 | Frame height (LE) — duplicate                     |
| 17     | 1    | uint8  | Frame index (0-based)                              |

For **static textures** (frame count = 1), there is exactly one frame descriptor. The X and Y
offsets are both 0, and the frame dimensions equal the total texture dimensions.

For **animated sprite sheets** (frame count > 1), each frame descriptor specifies where in the
atlas that frame is located. Frames are laid out in a grid within the atlas and enumerated in
reverse order (highest offset first, frame 0 is at the far end of the atlas, counting backwards
toward the origin).

### Trailing Bytes

After all frame descriptors:

| Version | Size | Description                                          |
|---------|------|------------------------------------------------------|
| T4X     | 2    | `0x00` padding byte + 1 content-dependent byte       |
| T3X     | 1    | `0x00` padding byte only (no content byte)           |

The content-dependent trailing byte in T4X varies:
- **`_combine` (diffuse)** textures: almost always `0xFE` (254) or `0xFF` (255)
- **`_ncombine*` (normal/material)** textures: typically `0x7F`–`0x99`
- **`_lightmask`** textures: variable (`0x00`–`0xFF`)

The exact meaning of this byte is not fully understood. It may be a content hash, average
luminance, or quality metric. It does not affect rendering.

### Total Header Size

```
T4X: 10 + (FrameCount × 18) + 2 bytes
T3X: 10 + (FrameCount × 18) + 1 bytes
```

For a standard static T4X texture (1 frame): `10 + 18 + 2 = 30 bytes`.

---

## Pixel Data

Pixel data begins immediately after the header and continues to the end of the file. The data
contains the complete mipmap chain for all levels, stored contiguously.

### Data Size Calculation

For **DXT1 (0x0C)**:
```
level_size(w, h) = max(1, w/4) × max(1, h/4) × 8 bytes

total = Σ level_size(w >> i, h >> i) for i in 0..mip_count-1
```

For **DXT5 (0x0E)**:
```
level_size(w, h) = max(1, w/4) × max(1, h/4) × 16 bytes

total = Σ level_size(w >> i, h >> i) for i in 0..mip_count-1
```

For **A8R8G8B8 (0x02)**:
```
level_size(w, h) = w × h × 4 bytes

total = Σ level_size(w >> i, h >> i) for i in 0..mip_count-1
```

### Xbox 360 Tiling (Swizzling)

The pixel data is stored in **Xbox 360 GPU tiled memory layout**, not in linear row-major order.
This is the native memory format used by the Xenos GPU and must be untiled (detiled) before the
image can be used on PC.

For DXT-compressed textures, the tiling operates on the level of DXT *blocks* (not individual
pixels). Each DXT block covers a 4×4 pixel region.

The Xbox 360 uses a two-level tiling scheme:

1. **Micro-tiles:** For DXT textures, a micro-tile is 8 blocks wide × 4 blocks tall
   (= 32×16 pixels). Blocks within a micro-tile are stored in linear row-major order.

2. **Macro-tiles:** Micro-tiles are arranged in a larger pattern across the texture surface
   using a Z-order (Morton) curve. The exact macro-tile dimensions depend on the texture
   format and pitch.

The `XGAddress2DTiledOffset()` function from the Xbox 360 XDK computes the tiled byte offset
for a given (x, y) coordinate. An equivalent implementation is needed to untile textures for
PC use.

#### Untiling Algorithm

For each DXT block at logical position (bx, by) in the mip level:

```c
// Compute the tiled offset for a block at (bx, by) in a surface
// that is 'width_blocks' blocks wide.
uint32_t tiled_offset(uint32_t bx, uint32_t by, uint32_t width_blocks, uint32_t bytes_per_block)
{
    // Micro-tile: 8 blocks wide, 4 blocks tall (for DXT)
    uint32_t macro_w = 8;  // blocks per micro-tile row
    uint32_t macro_h = 4;  // blocks per micro-tile column

    // Micro-tile position
    uint32_t mtx = bx / macro_w;
    uint32_t mty = by / macro_h;

    // Block within micro-tile
    uint32_t lbx = bx % macro_w;
    uint32_t lby = by % macro_h;

    // Micro-tiles per row
    uint32_t mt_per_row = (width_blocks + macro_w - 1) / macro_w;

    // Micro-tile index (row-major)
    uint32_t mt_index = mty * mt_per_row + mtx;

    // Offset within micro-tile (linear, row-major)
    uint32_t local_offset = (lby * macro_w + lbx) * bytes_per_block;

    // Final offset
    uint32_t mt_size = macro_w * macro_h * bytes_per_block;
    return mt_index * mt_size + local_offset;
}
```

**Note:** This is a simplified model. The full Xbox 360 tiling algorithm involves additional
complexity for textures with non-power-of-two dimensions and multi-sample surfaces. Testing
against known textures (e.g., `checker.tex`) is recommended for validation.

### Xbox 360 Byte Order (Endian Swap)

The Xbox 360 GPU is big-endian. DXT block data contains 16-bit values (color endpoints in
RGB565 format) that are stored in big-endian byte order. To use the data with a PC-side DXT
decoder, every 16-bit word (2-byte pair) in the pixel data must be byte-swapped.

This swap applies uniformly to the entire pixel data region — every consecutive pair of bytes
is reversed:

```c
void endian_swap_16(uint8_t* data, size_t size)
{
    for (size_t i = 0; i + 1 < size; i += 2)
    {
        uint8_t tmp = data[i];
        data[i] = data[i + 1];
        data[i + 1] = tmp;
    }
}
```

For uncompressed `A8R8G8B8` textures (format `0x02`), the endian swap should be applied as
32-bit words instead, since each pixel is a 4-byte ARGB value stored in big-endian order.

---

## Texture Naming Convention

Trials HD uses a multi-texture material system. Each visual asset has several texture maps,
distinguished by filename suffix:

| Suffix         | Format      | Description                                      |
|----------------|-------------|--------------------------------------------------|
| `_combine`     | DXT5 (0x0E) | Diffuse / albedo color map (RGB + alpha)         |
| `_ncombine1`   | DXT5 (0x0E) | Normal map channel 1 (tangent-space X/Y)         |
| `_ncombine2`   | DXT5 (0x0E) | Normal map channel 2                             |
| `_ncombine3`   | DXT5 (0x0E) | Material property map (specular / roughness)     |
| `_ncombine4`   | DXT5 (0x0E) | Material property map (ambient occlusion / etc.) |
| `_lightmask`   | DXT1 (0x0C) | Light attenuation mask (grayscale)               |

Not all objects use every channel. A minimal object has `_combine` + `_ncombine1` + `_ncombine2`.
Complex objects add `_ncombine3`, `_ncombine4`, and/or `_lightmask`.

### Animated Sprite Sheets

Particle effect textures use the `_group_` infix to indicate animated sprite sheets:

```
fire3_group_ncombine1.tex     1792×384, 40 frames of 128×128
lightning1_group_ncombine1.tex 1280×128, 10 frames of 128×128
animtest_group_ncombine1.tex   384×128,  3 frames of 128×128
```

Frames are packed into a 2D atlas grid. The total texture width and height are the atlas
dimensions, and each frame descriptor provides the (x, y) pixel offset of that frame within
the atlas.

---

## Worked Examples

### Example 1: Static 512×512 DXT5 Texture

**File:** `WWII_bomb_combine.tex` (349,550 bytes)

```
Header (30 bytes):
  [00-02] 54 34 58        Magic: "T4X"
  [03-04] 00 02           Width: 512
  [05-06] 00 02           Height: 512
  [07]    0e               Format: DXT5
  [08]    08               Mip levels: 8 (512→256→128→64→32→16→8→4)
  [09]    01               Frame count: 1

Frame Descriptor 0 (18 bytes, offset 10):
  [10]    00               Reserved
  [11-12] 00 00            X offset: 0
  [13-14] 00 00            Y offset: 0
  [15-18] 00 00 00 00      Reserved
  [19-20] 00 02            Frame width: 512
  [21-22] 00 02            Frame height: 512
  [23-24] 00 02            Frame width (dup): 512
  [25-26] 00 02            Frame height (dup): 512
  [27]    00               Frame index: 0

Trailing (2 bytes):
  [28]    00               Padding
  [29]    fe               Content byte

Pixel data (349,520 bytes, offset 30):
  Mip 0: 512×512 = 128×128 blocks × 16 = 262,144 bytes
  Mip 1: 256×256 =  64×64  blocks × 16 =  65,536 bytes
  Mip 2: 128×128 =  32×32  blocks × 16 =  16,384 bytes
  Mip 3:  64×64  =  16×16  blocks × 16 =   4,096 bytes
  Mip 4:  32×32  =   8×8   blocks × 16 =   1,024 bytes
  Mip 5:  16×16  =   4×4   blocks × 16 =     256 bytes
  Mip 6:   8×8   =   2×2   blocks × 16 =      64 bytes
  Mip 7:   4×4   =   1×1   blocks × 16 =      16 bytes
                                    Total: 349,520 bytes ✓
```

### Example 2: Animated Sprite Sheet (3 frames)

**File:** `animtest_group_ncombine1.tex` (65,586 bytes)

```
Header (66 bytes):
  [00-02] 54 34 58        Magic: "T4X"
  [03-04] 80 01           Width: 384 (= 3 × 128)
  [05-06] 80 00           Height: 128
  [07]    0e               Format: DXT5
  [08]    06               Mip levels: 6 (128→64→32→16→8→4)
  [09]    03               Frame count: 3

Frame Descriptor 0 (offset 10):
  X=256, Y=0, size=128×128, index=0

Frame Descriptor 1 (offset 28):
  X=128, Y=0, size=128×128, index=1

Frame Descriptor 2 (offset 46):
  X=0,   Y=0, size=128×128, index=2

Trailing (2 bytes, offset 64):
  [64] 00   [65] 7f

Pixel data (65,520 bytes, offset 66):
  384×128 atlas with 6 mip levels = 65,520 bytes ✓
```

### Example 3: Uncompressed T3X Texture

**File:** `noise.tex` (262,173 bytes)

```
Header (29 bytes):
  [00-02] 54 33 58        Magic: "T3X"
  [03-04] 00 01           Width: 256
  [05-06] 00 01           Height: 256
  [07]    02               Format: A8R8G8B8
  [08]    01               Mip levels: 1 (base only)
  [09]    00               Frame count: 0 (treated as 1 frame, no descriptors)

Frame Descriptor 0 (offset 10):
  X=0, Y=0, size=256×256, index=0

Trailing (1 byte, offset 28):
  [28] dc

Pixel data (262,144 bytes, offset 29):
  256 × 256 × 4 = 262,144 bytes ✓
```

**Note:** The T3X `noise.tex` has frame count = 0 and mips = 1, suggesting it uses a slightly
different convention where frame count 0 means "single frame, no frame descriptors." However,
the frame descriptor block is still present (it simply contains the default single-frame values).

---

## File Size Reference

| Dimensions  | Format | Mips | Frames | Header | Data Size   | Total Size  |
|-------------|--------|------|--------|--------|-------------|-------------|
| 4×4         | DXT5   | 1    | 1      | 30     | 16          | 46          |
| 64×64       | DXT5   | 5    | 1      | 30     | 5,456       | 5,486       |
| 128×128     | DXT1   | 6    | 1      | 30     | 10,920      | 10,950      |
| 128×128     | DXT5   | 6    | 1      | 30     | 21,840      | 21,870      |
| 256×256     | DXT1   | 7    | 1      | 30     | 43,688      | 43,718      |
| 256×256     | DXT5   | 7    | 1      | 30     | 87,376      | 87,406      |
| 512×512     | DXT1   | 8    | 1      | 30     | 174,760     | 174,790     |
| 512×512     | DXT5   | 8    | 1      | 30     | 349,520     | 349,550     |
| 1024×1024   | DXT1   | 9    | 1      | 30     | 699,048     | 699,078     |
| 1024×1024   | DXT5   | 9    | 1      | 30     | 1,398,096   | 1,398,126   |
| 128×128     | ARGB8  | 6    | 1      | 30     | 87,360      | 87,390      |
| 256×256     | ARGB8  | 1    | 1*     | 29     | 262,144     | 262,173     |
| 384×128     | DXT5   | 6    | 3      | 66     | 65,520      | 65,586      |
| 1280×128    | DXT5   | 6    | 10     | 192    | 218,400     | 218,592     |
| 1792×384    | DXT5   | 6    | 40     | 732    | 917,280     | 918,012     |

\* T3X format with frame count = 0

---

## Conversion Notes

### TEX → DDS (Export)

To convert a `.tex` file to a standard `.dds` file for use on PC:

1. **Read the header** — parse magic, dimensions, format, mip count, and frame count.
2. **Skip frame descriptors** — advance past all frame descriptor blocks and trailing bytes.
3. **Read pixel data** — the remainder of the file is the raw tiled pixel data.
4. **Untile the pixel data** — convert from Xbox 360 tiled layout to linear row-major order,
   processing each mip level independently.
5. **Byte-swap the pixel data** — swap every pair of bytes (16-bit words) for DXT formats,
   or every 4-byte group for ARGB8.
6. **Write a DDS header** — construct a standard DDS file header with the appropriate
   dimensions, format (DXT1/DXT5/A8R8G8B8), and mipmap count.
7. **Write the linear pixel data** — append the untiled, byte-swapped data after the DDS header.

### DDS → TEX (Import)

To convert a standard `.dds` file back to `.tex`:

1. **Read the DDS header** — extract dimensions, format, and mipmap count.
2. **Map the format** — DXT1 → 0x0C, DXT5 → 0x0E, A8R8G8B8 → 0x02.
3. **Byte-swap the pixel data** — convert from PC little-endian to Xbox 360 big-endian.
4. **Tile the pixel data** — convert from linear row-major to Xbox 360 tiled layout.
5. **Write the TEX header** — magic, dimensions, format, mip count, frame count (1 for simple
   textures), frame descriptor(s), and trailing bytes.
6. **Write the tiled pixel data** — append after the header.

### Notes on Sprite Sheet Handling

For animated sprite sheets, the pixel data covers the *entire atlas* (all frames composited
into a single large texture). The mipmap chain is generated for the atlas dimensions, not for
individual frames. Frame descriptors are metadata only — they describe where to find each frame
within the atlas but do not affect the pixel data layout.

When exporting a sprite sheet to DDS, the output is a single large texture containing all frames.
Individual frame extraction requires cropping based on the frame descriptors.