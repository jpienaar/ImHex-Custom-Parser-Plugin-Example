#include <hex/plugin.hpp>

#include <hex/api/content_registry/views.hpp>
#include <hex/api/content_registry/pattern_language.hpp>
#include <hex/helpers/logger.hpp>
#include <romfs/romfs.hpp>

#include "views/view_myformat.hpp"

using namespace hex;

namespace hex::plugin::myformat {
    // Forward declaration from pl_myformat_type.cpp
    void registerPatternLanguageType();
}

namespace {

    void registerViews() {
        hex::log::debug("Registering MyFormat Viewer");
        ContentRegistry::Views::add<hex::plugin::myformat::ViewMyFormat>();
    }

}

IMHEX_PLUGIN_SETUP("MyFormat Parser", "0.0.1", "MyFormat Parser Plugin") {
    hex::log::debug("Initializing MyFormat Parser Plugin");

    
    // Register the custom pattern language type and functions
    hex::plugin::myformat::registerPatternLanguageType();
    
    // Register the custom view
    registerViews();
    hex::log::info("MyFormat Parser plugin loaded successfully");
}
