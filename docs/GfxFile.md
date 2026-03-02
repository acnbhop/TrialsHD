# GFX File Format (.gfx)

The `.gfx` file format is the serialized scene graph used by Trials HD's RedLynx engine. Each `.gfx` file
stores a tree of scene nodes — meshes, lights, sounds, particle emitters, collision shapes, sub-scene
references, and skeletal rigs — together with their transforms, properties, and material/controller data.

All multi-byte values are **little-endian**.

---

## High-Level Layout

```
[4 bytes]   Version (uint32, always 1)
[N bytes]   Root node block (recursive, self-describing)
[1 byte]    Post-data flag (always 0x01)
[4 bytes]   Magic marker: EF BE EF BE
[4 bytes]   Post-data size (uint32, size of the post-data payload)
[M bytes]   Post-data payload (materials, animation controllers)
[8 bytes]   Footer (file-level trailing data)
```

---

## Version

| Offset | Type   | Description           |
|--------|--------|-----------------------|
| 0      | uint32 | File version, always `1` |

---

## Node Block (Recursive)

After the version, the file is a depth-first serialization of a node tree. Each node has:

1. **Node-type-specific property data** (varies per node type, floats + bytes + integers)
2. **Instance hash** (uint32) — unique per node instance, `0` for the root
3. **Type hash** (uint32) — identifies the node class (see table below)
4. **ID string** (length-prefixed) — a numeric or short identifier string
5. **Name string** (length-prefixed) — the human-readable node name
6. **Node-type-specific post-name data** (additional properties)
7. **Child nodes** (recursively serialized)

### Root Node

The root node has a special layout. After the 4-byte version, the root's property data begins
immediately (typically 84 bytes of transform/bounding data), followed by:

| Offset | Type   | Description               |
|--------|--------|---------------------------|
| 88     | uint32 | Instance hash (always `0`) |
| 92     | uint32 | Type hash                  |
| 96     | uint32 | ID string length (always `0` for root) |
| 100    | uint32 | Type name string length    |
| 104    | char[] | Type name (e.g. `"SceneObject"`) |

For `SceneObject` roots, the type hash is `0x73D39136`.
For rig files, the type hash is `0x00000000` and the type name is empty.

### Length-Prefixed Strings

Strings in `.gfx` files are stored as:

| Field  | Type   | Description            |
|--------|--------|------------------------|
| Length | uint32 | Number of ASCII bytes  |
| Data   | char[] | ASCII string (no null terminator, no padding) |

### Known Type Hashes

| Hash         | Node Type     | Description                              |
|--------------|---------------|------------------------------------------|
| `0x73D39136` | SceneObject   | Root scene container                     |
| `0x65793C0C` | Mesh          | Visual mesh with model path              |
| `0x9E0A6A8E` | Box           | Box collision shape                      |
| `0xDC21446C` | Sphere        | Sphere collision shape                   |
| `0x7B250B76` | Sphere (alt)  | Alternative sphere type                  |
| `0x2FF3AA15` | Sound         | Sound emitter with XACT reference        |
| `0x0D27FA8C` | Light         | Dynamic light source                     |
| `0xC72C550B` | FireSource    | Fire/particle emitter source             |
| `0x03988A53` | Serialized    | Sub-scene reference (.gfx path)          |
| `0x00000000` | Rig           | Skeletal rig (driver_rig, etc.)          |

**Particle nodes** (e.g. Fire, Smoke, Sparks) use the same value for both instance hash and type hash.
This appears to be a content-hashed identifier rather than a class hash.

---

## Root Property Data (Offset 4–87)

The root node begins with 84 bytes of transform and bounding data immediately after the version field.
These are interpreted as little-endian floats and integers:

| Offset | Size    | Description                                      |
|--------|---------|--------------------------------------------------|
| 4      | uint32  | Root type flags / node property code (e.g. `13`)  |
| 8–55   | 12×f32  | Bounding volume data (center, extents, min, max)  |
| 56–67  | 3×f32   | Scale (typically `1.0, 1.0, 1.0`)                 |
| 68–79  | 3×f32   | Rotation (euler or quaternion XYZ components)      |
| 80–83  | f32     | Rotation W (quaternion, `1.0` = identity)          |
| 84–87  | f32     | Reserved / extra parameter                         |

---

## EFBEEFBE Marker (Post-Data Section)

The scene tree ends with the byte `0x01` followed by the 4-byte magic `EF BE EF BE`. This marks the
start of the post-data section which contains material definitions and animation controller curves.

```
[01]            Post-data present flag
[EF BE EF BE]   Magic marker
[uint32]        Post-data payload size (in bytes)
[uint32]        Material/rendering flags
[uint32]        Material name string length
[char[]]        Material name string (e.g. "default")
[...]           Material properties (LOD distances, alpha, etc.)
[...]           Animation controller data (ParticleColorA, ParticleSize, etc.)
```

### Material Block

After the EFBEEFBE marker and size field:

| Field           | Type   | Description                     |
|-----------------|--------|---------------------------------|
| Flags           | uint32 | Material/rendering flags        |
| Name length     | uint32 | Material name string length     |
| Name            | char[] | Material name (e.g. `"default"`) |
| LOD distances   | f32×2  | LOD near/far distances (e.g. `100.0, 100.0`) |
| Alpha           | f32    | Material alpha (e.g. `1.0`)     |
| Padding         | varies | Trailing zeros                  |

### Animation Controllers

For files with particle effects or animated properties, the post-data section also contains
serialized animation curves. Each controller has:

| Field         | Type   | Description                            |
|---------------|--------|----------------------------------------|
| Flag          | uint8  | Always `0x01`                          |
| Name length   | uint32 | Controller name length                 |
| Name          | char[] | Controller name (e.g. `"ParticleColorA"`) |
| Curve type    | uint8  | Curve interpolation type               |
| Curve flags   | uint8  | Additional flags                       |
| Key count     | uint32 | Number of keyframes                    |
| Keys          | varies | Keyframe data (time + value + tangents) |

Known controller names:
- `ParticleColorA` — particle color alpha over lifetime
- `ParticleSize` — particle size over lifetime
- `ParticleRotation` — particle rotation over lifetime
- `ParticleFrame` — particle texture frame over lifetime
- `m_intensity` — light intensity curve
- `Light1` — light property curve

---

## Footer

The final 8 bytes of the file (for SceneObject-rooted files) contain file-level trailing data,
typically including hash references and zero padding.

---

## XML Round-Trip Format

The `gfx_tool` exports `.gfx` files to XML for inspection and byte-accurate round-tripping.
The XML format stores the entire file as a hex-encoded data blob, with parsed node metadata
shown as annotations:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<GfxFile>
    <Version>1</Version>
    <Data>01000000...</Data>
    <Nodes>
        <Node index="0" offset="256" instanceHash="01B68B0D"
              typeHash="65793C0C" type="Mesh" id="187508880" name="Mesh0"/>
        ...
    </Nodes>
</GfxFile>
```

The `<Data>` element contains the complete file as a hex string. On import, it is
decoded directly back to binary, ensuring byte-perfect reconstruction. The `<Nodes>`
element is metadata-only and is not used during import.

---

## File Size Reference

| File Type          | Typical Size  | Description                     |
|--------------------|---------------|---------------------------------|
| Simple mesh object | 300–700 bytes | Single mesh + material          |
| Group/composite    | 400–800 bytes | Sub-scene references            |
| Particle effect    | 2000–5000 bytes | Emitters + controllers         |
| Bike scene         | 10000–25000 bytes | Complex scene with physics    |
| Driver rig         | 8000–9000 bytes | Skeletal rig with bones        |
