cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
project(MyFormatImHexPlugin)

# Include the ImHex SDK
# For this to work, you need to set the IMHEX_SDK_PATH environment variable to the path of the ImHex SDK
#
# On Windows, the SDK is next to the ImHex executable
# On Linux, the SDK is usually in /usr/share/imhex/sdk but this may vary depending on your distribution
# On MacOS, the SDK is located inside of the ImHex.app bundle under ImHex.app/Contents/Resources/sdk
include(ImHexSDK)

add_imhex_plugin(
    NAME
        myformat_parser
    SOURCES
        source/plugin_myformat.cpp
        source/content/pl_myformat_type.cpp
        source/content/views/view_myformat.cpp
    INCLUDES
        include
    LIBRARY
        ui
        fonts
)
