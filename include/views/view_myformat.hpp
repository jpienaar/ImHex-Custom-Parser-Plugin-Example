#pragma once

#include <hex/ui/view.hpp>
#include "myformat_parser.hpp"

namespace hex::plugin::myformat {

// Custom View for visualizing MyFormat files
// 
// This view provides specialized visualization beyond what the
// pattern language can offer, such as summary statistics, graphs, etc.
class ViewMyFormat : public View::Window {
public:
    ViewMyFormat();
    ~ViewMyFormat() override = default;

    void drawContent() override;
    void drawHelpText() override;
    
private:
    void parseCurrentFile();
    void drawHeader();
    void drawArrays();
    void drawSummary();
    
    ::myformat::ParsedData m_parsedData;
    bool m_needsReparse = true;
};

}  // namespace hex::plugin::myformat
