#include "views/view_myformat.hpp"
#include <hex/helpers/logger.hpp>
#include <hex/helpers/fmt.hpp>

#include <hex/api/imhex_api/provider.hpp>
#include <hex/providers/provider.hpp>

#include <imgui.h>

// Repeated here as fonts not in system path.
#define ICON_VS_ERROR "\xee\xaa\x87"	// U+EA87
#define ICON_VS_FILE_BINARY "\xee\xab\xa8"	// U+EAE8

namespace hex::plugin::myformat {

    ViewMyFormat::ViewMyFormat() 
        : View::Window("MyFormat Viewer", ICON_VS_FILE_BINARY) 
    {
        hex::log::info("ViewMyFormat initialized");
        // Mark that we need to reparse when provider changes
        EventProviderOpened::subscribe(this, [this]([[maybe_unused]] prv::Provider* provider) {
            m_needsReparse = true;
        });
        
        EventProviderClosed::subscribe(this, [this]([[maybe_unused]] prv::Provider* provider) {
            m_parsedData = ::myformat::ParsedData{};
            m_needsReparse = true;
        });
    }

    void ViewMyFormat::parseCurrentFile() {
        hex::log::debug("ViewMyFormat::parseCurrentFile() called");
        m_parsedData = ::myformat::ParsedData{};
        
        auto provider = ImHexApi::Provider::get();
        if (!provider || !provider->isAvailable()) {
            return;
        }
        
        // Read the file data
        size_t fileSize = provider->getActualSize();
        if (fileSize < 12) {
            return;  // Too small for header
        }
        
        // Limit read size for safety
        constexpr size_t maxReadSize = 1024 * 1024;  // 1 MB
        size_t readSize = std::min(fileSize, maxReadSize);
        
        std::vector<u8> data(readSize);
        provider->read(0, data.data(), readSize);
        
        // Parse using the C++ parser
        m_parsedData = ::myformat::Parser::parse(data.data(), data.size());
        m_needsReparse = false;
    }

    void ViewMyFormat::drawHelpText() {
        ImGuiExt::TextFormattedWrapped("Specialized viewer for MyFormat files including array visualization, and summary statistics.");
    }

    void ViewMyFormat::drawContent() {
        // Reparse if needed
        if (m_needsReparse) {
            parseCurrentFile();
        }
        
        if (!m_parsedData.valid) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), 
                ICON_VS_ERROR " Not a valid MyFormat file");
            ImGui::TextWrapped("This view shows files with the MyFormat structure "
                "(magic 'MYFM', two counts, two arrays).");
            return;
        }
        
        drawHeader();
        ImGui::Separator();
        drawArrays();
        ImGui::Separator();
        drawSummary();
    }

    void ViewMyFormat::drawHeader() {
        if (ImGui::CollapsingHeader("Header", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            
            ImGui::Text("Magic:  0x%08X", m_parsedData.magic);
            ImGui::SameLine();
            
            // Show magic as ASCII
            char magicStr[5] = {0};
            std::memcpy(magicStr, &m_parsedData.magic, 4);
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "(\"%s\")", magicStr);
            
            ImGui::Text("Count1: %u", m_parsedData.count1);
            ImGui::Text("Count2: %u", m_parsedData.count2);
            ImGui::Text("Total Size: %zu bytes", m_parsedData.totalSize);
            
            ImGui::Unindent();
        }
    }

    void ViewMyFormat::drawArrays() {
        // Array 1
        if (ImGui::CollapsingHeader(
            fmt::format("Array 1 ({} elements)###array1", m_parsedData.array1.size()).c_str(),
            ImGuiTreeNodeFlags_DefaultOpen
        )) {
            ImGui::Indent();
            
            if (m_parsedData.array1.empty()) {
                ImGui::TextDisabled("(empty)");
            } else {
                // Use a table for better display
                if (ImGui::BeginTable("Array1Table", 4, 
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                    ImVec2(0, 150)
                )) {
                    ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 60);
                    ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed, 100);
                    ImGui::TableSetupColumn("Decimal", ImGuiTableColumnFlags_WidthFixed, 100);
                    ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, 80);
                    ImGui::TableHeadersRow();
                    
                    for (size_t i = 0; i < m_parsedData.array1.size(); i++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%zu", i);
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%08X", m_parsedData.array1[i]);
                        ImGui::TableNextColumn();
                        ImGui::Text("%u", m_parsedData.array1[i]);
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%zX", 12 + i * 4);
                    }
                    
                    ImGui::EndTable();
                }
            }
            
            ImGui::Unindent();
        }
        
        // Array 2
        if (ImGui::CollapsingHeader(
            fmt::format("Array 2 ({} elements)###array2", m_parsedData.array2.size()).c_str(),
            ImGuiTreeNodeFlags_DefaultOpen
        )) {
            ImGui::Indent();
            
            if (m_parsedData.array2.empty()) {
                ImGui::TextDisabled("(empty)");
            } else {
                size_t array2Offset = 12 + m_parsedData.array1.size() * 4;
                
                if (ImGui::BeginTable("Array2Table", 4,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                    ImVec2(0, 150)
                )) {
                    ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 60);
                    ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed, 100);
                    ImGui::TableSetupColumn("Decimal", ImGuiTableColumnFlags_WidthFixed, 100);
                    ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, 80);
                    ImGui::TableHeadersRow();
                    
                    for (size_t i = 0; i < m_parsedData.array2.size(); i++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%zu", i);
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%08X", m_parsedData.array2[i]);
                        ImGui::TableNextColumn();
                        ImGui::Text("%u", m_parsedData.array2[i]);
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%zX", array2Offset + i * 4);
                    }
                    
                    ImGui::EndTable();
                }
            }
            
            ImGui::Unindent();
        }
    }

    void ViewMyFormat::drawSummary() {
        if (ImGui::CollapsingHeader("Summary Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            
            // Calculate some statistics
            auto calcStats = [](const std::vector<u32>& arr) {
                if (arr.empty()) return std::make_tuple(0u, 0u, 0.0);
                u32 minVal = *std::min_element(arr.begin(), arr.end());
                u32 maxVal = *std::max_element(arr.begin(), arr.end());
                double sum = 0;
                for (auto v : arr) sum += v;
                double avg = sum / arr.size();
                return std::make_tuple(minVal, maxVal, avg);
            };
            
            auto [min1, max1, avg1] = calcStats(m_parsedData.array1);
            auto [min2, max2, avg2] = calcStats(m_parsedData.array2);
            
            ImGui::Text("Array 1: min=%u, max=%u, avg=%.2f", min1, max1, avg1);
            ImGui::Text("Array 2: min=%u, max=%u, avg=%.2f", min2, max2, avg2);
            
            ImGui::Unindent();
        }
    }

}  // namespace hex::plugin::myformat
