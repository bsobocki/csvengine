add_library(csvengine
    src/csvparser/csvparser.cpp
    src/csvparser/simple/csvparser_simple_parser.cpp
    src/csvparser/simple/csvparser_simd_parser.cpp
    src/csvparser/quoting/csvparser_quoting_strict_parser.cpp
    src/csvparser/quoting/csvparser_quoting_lenient_parser.cpp
    src/csvreader.cpp
    src/csvmappedbuffer.cpp
)

# public headers for the 'csvengine' library are located in the 'inc' directory.
target_include_directories(csvengine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
