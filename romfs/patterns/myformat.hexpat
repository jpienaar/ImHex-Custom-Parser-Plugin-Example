/**
 * MyFormat Pattern File
 * 
 * This pattern uses the custom C++ parser registered by the myformat_parser plugin.
 * 
 * Usage:
 * 1. Build ImHex with the myformat_parser plugin
 * 2. Open a MyFormat file (starts with "MYFM" magic)
 * 3. Load this pattern file
 * 
 * You can also use this directly in ImHex's pattern editor.
 */

#pragma description MyFormat file with magic, counts, and variable-length arrays
#pragma endian little
#pragma magic [ 4D 59 46 4D ] @ 0x00

// Note: 'myformat' namespace is automatically provided by the C++ plugin.
// No import statement is needed for types registered in C++.



// Use the custom parser to create the pattern hierarchy
myformat::MyFormat file @ 0x00;
