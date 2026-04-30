add_library(csvengine
    src/csvparser/csvparser.cpp
    src/csvparser/simple/csvparser_simpleparser.cpp
    src/csvparser/simple/csvparser_simpleparserbase.cpp
    src/csvparser/simple/csvparser_simdparser.cpp
    src/csvparser/simple/csvparser_simdparserbase.cpp
    src/csvparser/simple/csvviewparser_simpleparser.cpp
    src/csvparser/simple/csvviewparser_simdparser.cpp
    src/csvparser/quoting/csvparser_strictquotingparser.cpp
    src/csvparser/quoting/csvparser_lenientquotingparser.cpp
    src/csvreader/csvreader.cpp
    src/csvreader/csvreaderbase.cpp
    src/csvreader/csvviewreader.cpp
    src/csvmappedbuffer.cpp
)

# public headers for the 'csvengine' library are located in the 'inc' directory.
target_include_directories(csvengine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-msse2" COMPILER_SUPPORTS_SSE2)
if(COMPILER_SUPPORTS_SSE2)
    target_compile_options(csvengine PRIVATE -msse2)
endif()
