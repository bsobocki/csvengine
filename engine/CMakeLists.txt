add_library(csvengine
    src/csvparser.cpp
    src/csvparser_simple_parser.cpp
    src/csvparser_quoting_strict_parser.cpp
    src/csvparser_quoting_lenient_parser.cpp
    src/csvreader.cpp
)

# public headers for the 'csvengine' library are located in the 'inc' directory.
target_include_directories(csvengine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
