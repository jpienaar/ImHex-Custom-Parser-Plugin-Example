#include <hex/api/content_registry/pattern_language.hpp>
#include <hex/helpers/logger.hpp>

#include <pl/core/evaluator.hpp>
#include <pl/core/errors/error.hpp>

#include <pl/patterns/pattern.hpp>
#include <pl/patterns/pattern_struct.hpp>
#include <pl/patterns/pattern_array_dynamic.hpp>
#include <pl/patterns/pattern_unsigned.hpp>
#include <pl/patterns/pattern_string.hpp>

#include "myformat_parser.hpp"

namespace hex::plugin::myformat {

    // This is the key function that bridges your existing C++ parser with ImHex's
    // pattern language visualization system.
    static std::shared_ptr<pl::ptrn::Pattern> createPatternFromParsedData(
        pl::core::Evaluator *evaluator,
        const ::myformat::ParsedData& data,
        u64 baseOffset
    ) {
        // Create the root struct pattern
        auto root = std::make_shared<pl::ptrn::PatternStruct>(
            evaluator, baseOffset, data.totalSize, 0
        );
        root->setTypeName("MyFormat");
        root->setVariableName("file");
        
        std::vector<std::shared_ptr<pl::ptrn::Pattern>> members;
        u64 offset = baseOffset;
        
        // Magic field (4 bytes)
        auto pMagic = std::make_shared<pl::ptrn::PatternString>(
            evaluator, offset, 4, 0
        );
        pMagic->setVariableName("magic");
        pMagic->setTypeName("char[]");
        members.push_back(pMagic);
        offset += 4;
        
        // count1 field (4 bytes)
        auto pCount1 = std::make_shared<pl::ptrn::PatternUnsigned>(
            evaluator, offset, sizeof(u32), 0
        );
        pCount1->setVariableName("count1");
        pCount1->setTypeName("u32");
        members.push_back(pCount1);
        offset += sizeof(u32);
        
        // count2 field (4 bytes)
        auto pCount2 = std::make_shared<pl::ptrn::PatternUnsigned>(
            evaluator, offset, sizeof(u32), 0
        );
        pCount2->setVariableName("count2");
        pCount2->setTypeName("u32");
        members.push_back(pCount2);
        offset += sizeof(u32);
        
        // array1 - dynamic array sized by count1.
        if (!data.array1.empty()) {
            auto pArray1 = std::make_shared<pl::ptrn::PatternArrayDynamic>(
                evaluator, offset, pCount1->getValue().toUnsigned() * sizeof(u32), 0
            );
            pArray1->setVariableName("array1");
            pArray1->setTypeName("u32");
            
            std::vector<std::shared_ptr<pl::ptrn::Pattern>> array1Entries;
            for (size_t i = 0; i < data.array1.size(); i++) {
                auto elem = std::make_shared<pl::ptrn::PatternUnsigned>(
                    evaluator, offset + i * sizeof(u32), sizeof(u32), 0
                );
                elem->setArrayIndex(i);
                elem->setTypeName("u32");
                array1Entries.push_back(elem);
            }
            pArray1->setEntries(array1Entries);
            members.push_back(pArray1);
            offset += data.array1.size() * sizeof(u32);
        }
        
        // array2 - dynamic array sized by count2
        if (!data.array2.empty()) {
            auto pArray2 = std::make_shared<pl::ptrn::PatternArrayDynamic>(
                evaluator, offset, data.array2.size() * sizeof(u32), 0
            );
            pArray2->setVariableName("array2");
            pArray2->setTypeName("u32");
            
            std::vector<std::shared_ptr<pl::ptrn::Pattern>> array2Entries;
            for (size_t i = 0; i < data.array2.size(); i++) {
                auto elem = std::make_shared<pl::ptrn::PatternUnsigned>(
                    evaluator, offset + i * sizeof(u32), sizeof(u32), 0
                );
                elem->setArrayIndex(i);
                elem->setTypeName("u32");
                array2Entries.push_back(elem);
            }
            pArray2->setEntries(array2Entries);
            members.push_back(pArray2);
        }
        
        root->setEntries(members);
        return root;
    }

    // Register the custom pattern language type.
    // This makes `myformat::MyFormat` available in pattern language scripts.
    // When used in a .hexpat file, it will invoke your C++ parser and create
    // the appropriate pattern hierarchy for visualization.
    void registerPatternLanguageType() {
        hex::log::info("Registering MyFormat pattern language type and functions");
        using namespace pl::core;
        using FunctionParameterCount = pl::api::FunctionParameterCount;
        
        const pl::api::Namespace ns = { "myformat" };
        
        // Usage in pattern: myformat::MyFormat file @ 0x00;
        ContentRegistry::PatternLanguage::addType(
            ns, 
            "MyFormat", 
            FunctionParameterCount::none(),  // No parameters needed - we read size from file
            [](Evaluator *evaluator, [[maybe_unused]] auto params) -> std::shared_ptr<pl::ptrn::Pattern> {
                hex::log::debug("myformat::MyFormat type callback triggered");
                u64 offset = evaluator->getReadOffset();
                u64 dataSize = evaluator->getDataSize();
                
                // Calculate how much we can read
                u64 availableSize = dataSize - offset;
                if (availableSize < 12) {
                    err::E0012.throwError("File too small for MyFormat header (need at least 12 bytes)");
                }
                
                // Read all available data (up to a reasonable limit)
                constexpr u64 maxReadSize = 1024 * 1024;  // 1 MB limit
                u64 readSize = std::min(availableSize, maxReadSize);
                
                std::vector<u8> buffer(readSize);
                evaluator->readData(offset, buffer.data(), readSize, pl::ptrn::Pattern::MainSectionId);
                
                // Use the existing C++ parser
                auto parsedData = ::myformat::Parser::parse(buffer.data(), buffer.size());
                
                if (!parsedData.valid) {
                    err::E0012.throwError("Failed to parse MyFormat: invalid magic or structure");
                }
                
                // Convert to pattern hierarchy
                return createPatternFromParsedData(evaluator, parsedData, offset);
            }
        );
        
        // Also register a helper function to get array counts
        // Usage: myformat::get_count(0) returns count1, myformat::get_count(1) returns count2
        ContentRegistry::PatternLanguage::addFunction(
            ns,
            "get_count",
            FunctionParameterCount::exactly(1),
            [](Evaluator *evaluator, auto params) -> std::optional<Token::Literal> {
                u64 index = params[0].toUnsigned();
                
                // Read header
                u8 header[12];
                evaluator->readData(0, header, 12, pl::ptrn::Pattern::MainSectionId);
                
                auto parsed = ::myformat::Parser::parse(header, 12);
                if (!parsed.valid) {
                    return static_cast<u128>(0);
                }
                
                return index == 0 ? static_cast<u128>(parsed.count1) : static_cast<u128>(parsed.count2);
            }
        );

    }

}  // namespace hex::plugin::myformat
