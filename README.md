# MyFormat Parser Plugin

An ImHex plugin demonstrating **two distinct approaches** for integrating an existing C++ parser:

1. **Pattern Language Type** — Register a custom type usable in `.hexpat` files
2. **Custom View** — Create a dedicated UI panel with specialized visualization

You can use either approach independently, or combine them as shown here.

In general one would want to use the pattern language type approach as it
integrates with the existing ImHex tooling. The custom view approach is only
necessary when the pattern language type approach is not sufficient for the
needs of the plugin. And even then this shows an extreme example where the
entire parser is implemented in C++ and then exposed via the pattern language
type approach. This need not encompass the entire file and could instead only
be used for specific parts of the file/custom types calling out.

## Overview & Tradeoffs

| Approach | Complexity | Limitations |
|----------|------------|-------------|
| **Pattern Language Type** | Low | Limited to tree view, no custom charts |
| **Custom View** | Medium | Doesn't integrate with Pattern Data view |
| **Hybrid (both)** | Medium | More code to maintain |

### When to Use Each

- **Pattern Language Type only**: You want parsed data to appear in the Pattern
  Data view and integrate with ImHex's existing tooling.

- **Custom View only**: You need specialized visualization (tables, graphs,
  statistics) but don't need pattern language integration.

- **Hybrid**: Users can choose their preferred interface, or use both for
  different purposes.


## File Format Specification

This example implements the Hybrid approach for a simple custom format
that consists of a header followed by two variable-length arrays:

| Offset | Size | Name | Type | Description |
|--------|------|------|------|-------------|
| 0x00 | 4 | `magic` | char[4] | Magic bytes: `MYFM` |
| 0x04 | 4 | `count1` | u32 | Number of elements in `array1` |
| 0x08 | 4 | `count2` | u32 | Number of elements in `array2` |
| 0x0C | `count1`*4 | `array1` | u32[] | First data array |
| Varies | `count2`*4 | `array2` | u32[] | Second data array (starts after `array1`) |

> [!NOTE]
> All integers are in **little-endian** byte order.

---

## Usage

### Pattern Language

```cpp
#pragma magic [ 4D 59 46 4D ] @ 0x00  // \"MYFM\" magic - enables auto-detection

// Parse the file - magic already validated by #pragma
// Note: 'myformat' namespace is provided directly by the C++ plugin
myformat::MyFormat file @ 0x00;
```

### Helper Functions

| Function | Description |
|----------|-------------|
| `myformat::get_count(0)` | Returns count1 (first array size) |
| `myformat::get_count(1)` | Returns count2 (second array size) |

### Custom View

Open via **View → MyFormat Viewer**. Provides:
- Header summary (magic, counts, total size)
- Array tables with index, hex, decimal, and offset columns
- Summary statistics (min, max, average per array)

## Adapting for Your Parser

1. **Replace** `include/myformat_parser.hpp` with your C++ parser header.

2. **Modify** `createPatternFromParsedData()` in `pl_myformat_type.cpp`:
   This function is the bridge between your parser's output and ImHex's tree
   view. You must map your C++ fields to ImHex pattern objects. For example,
   create the following mappings:

   | ImHex Type | Usage |
   |------------|-------|
   | `pl::ptrn::PatternUnsigned<u32>` | For 32-bit unsigned integers |
   | `pl::ptrn::PatternArrayDynamic` | For arrays where the size is known from a previous field |
   | `pl::ptrn::PatternStruct` | To group related fields into a collapsible node |

   **Key Strategy**: The C++ parser should calculate all offsets and sizes. The
   Pattern Language wrapper then simply replicates those findings into the ImHex
   visual tree using `addMember()`.

3. **Update** `ViewMyFormat` for your custom UI:
   - Use `m_parsedData` (populated by your parser) as the data source.
   - Leverage `ImGui` for rendering tables, graphs, or specialized visualizations that don't fit in a tree view.

## Source Files

| File | Purpose |
|------|---------|
| `include/myformat_parser.hpp` | C++ parser (example - replace with yours) |
| `source/content/pl_myformat_type.cpp` | Pattern language type registration |
| `source/content/views/view_myformat.cpp` | Custom View implementation |
| `source/plugin_myformat.cpp` | Plugin entry point |
| `romfs/patterns/myformat.hexpat` | Example pattern file |

## License

Same as ImHex (GPLv2).
