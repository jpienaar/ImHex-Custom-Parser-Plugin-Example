#pragma once

#include <cstdint>
#include <vector>
#include <cstring>

namespace myformat {

// Very simple example file format and simplistic reader.

/* File format structure:

    Offset  Size    Field
    0x00    4       Magic ("MYFM" = 0x4D59464D)
    0x04    4       count1 (u32, little-endian)
    0x08    4       count2 (u32, little-endian)
    0x0C    4*N     array1[count1]
    ...     4*M     array2[count2]
 */

constexpr uint32_t MAGIC = 0x4D46594D;  // "MYFM" in little-endian (Bytes: 4D 59 46 4D)

struct ParsedData {
    bool valid = false;
    uint32_t magic = 0;
    uint32_t count1 = 0;
    uint32_t count2 = 0;
    std::vector<uint32_t> array1;
    std::vector<uint32_t> array2;
    size_t totalSize = 0;
};

// Example C++ parser that you would replace with your existing parser.
class Parser {
public:
    static ParsedData parse(const uint8_t* data, size_t size) {
        ParsedData result;
        
        // Need at least header: magic + count1 + count2 = 12 bytes
        if (size < 12) {
            return result;
        }
        
        // Read header
        std::memcpy(&result.magic, data, 4);
        std::memcpy(&result.count1, data + 4, 4);
        std::memcpy(&result.count2, data + 8, 4);
        
        // Validate magic
        if (result.magic != MAGIC) {
            return result;
        }
        
        // Calculate required size
        size_t requiredSize = 12 + (result.count1 + result.count2) * 4;
        if (size < requiredSize) {
            return result;
        }
        
        // Read array1
        result.array1.resize(result.count1);
        for (int32_t i = 0, e = result.count1; i < e; i++) {
            std::memcpy(&result.array1[i], data + 12 + i * 4, 4);
        }
        
        // Read array2
        size_t array2Offset = 12 + result.count1 * 4;
        result.array2.resize(result.count2);
        for (int32_t i = 0, e = result.count2; i < e; i++) {
            std::memcpy(&result.array2[i], data + array2Offset + i * 4, 4);
        }
        
        result.totalSize = requiredSize;
        result.valid = true;
        return result;
    }
    
    static bool validateMagic(const uint8_t* data, size_t size) {
        if (size < 4) return false;
        uint32_t magic;
        std::memcpy(&magic, data, 4);
        return magic == MAGIC;
    }
};

}  // namespace myformat
