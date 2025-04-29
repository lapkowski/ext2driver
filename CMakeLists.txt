cmake_minimum_required(VERSION 3.12)

find_program(CCACHE ccache)
if(CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

project(Ext2Driver VERSION 0.1.0
                   DESCRIPTION "A simple ext2 driver"
                   LANGUAGES CXX)

find_program(CLANG_FORMAT clang-format)
find_program(CPPCHECK cppcheck)
find_program(VALGRIND valgrind)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MAIN_SOURCE src/driver.cpp)

set(SOURCES
    src/filesystem.cpp
    src/inode.cpp)

set(HEADERS
    src/filesystem.hpp
    src/inode.hpp
    src/helpers.hpp)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(_DEBUG)
endif()

configure_file(
  "${CMAKE_SOURCE_DIR}/src/config.hpp.in"
  "${CMAKE_BINARY_DIR}/config.hpp")

include_directories(${CMAKE_BINARY_DIR})

add_executable(ext2_driver ${SOURCES} ${MAIN_SOURCE})

if(CLANG_FORMAT)
  add_custom_target(
    format
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND clang-format -i ${SOURCES} ${HEADERS} ${MAIN_SOURCE} ${TEST_SOURCES})
endif()

if(CPPCHECK)
  add_custom_target(
    lint 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND cppcheck 
            --enable=all 
            --inconclusive 
            --std=c++20 
            --suppress=missingIncludeSystem 
            --suppress=unmatchedSuppression
            --suppress=missingInclude
            --suppress=internalAstError
            ${SOURCES} ${HEADERS} ${MAIN_SOURCE})
endif()

install(TARGETS ext2_driver DESTINATION bin)
