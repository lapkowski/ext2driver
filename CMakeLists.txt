cmake_minimum_required(VERSION 3.12)

find_program(CCACHE ccache)
if(CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

project(Ext2Driver VERSION 0.1.0
                   DESCRIPTION "A simple ext2 driver"
                   LANGUAGES CXX)

include(CTest)

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

set(TEST_SOURCES
  tests/read.test.cpp)

set(MKE2FS "/sbin/mke2fs" CACHE STRING "Path to the mke2fs binary")
set(USE_DEV_SHM OFF CACHE BOOL "Use the /dev/shm filesystem for testing. Filesystem will be copied on failure.")

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

option(TESTS "Build tests" ON)

if (TESTS)
    find_package(GTest)

    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG main
        )

        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(googletest)
    endif()

    find_package(nlohmann_json 3.12.0 QUIET)

    if(NOT nlohmann_json_FOUND)
      include(FetchContent)
      message(STATUS "nlohmann_json was not found, building it from source...")
      FetchContent_Declare(nlohmann_json
	GIT_REPOSITORY https://github.com/nlohmann/json
	GIT_TAG v3.12.0)
      FetchContent_MakeAvailable(nlohmann_json)
    else()
      find_package(nlohmann_json 3.12.0 REQUIRED)
    endif()

    enable_testing()

    add_executable(testexe ${TEST_SOURCES} ${SOURCES})
    target_link_libraries(testexe GTest::gtest_main)
    target_link_libraries(testexe nlohmann_json::nlohmann_json)
    target_include_directories(testexe PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_include_directories(testexe PRIVATE ${CMAKE_SOURCE_DIR}/include)

    include(GoogleTest)
    if (USE_DEV_SHM)
      string(RANDOM postfix)
      set(image_dir "/dev/shm/ext2driver_test_${postfix}")
    else()
      set(image_dir "${CMAKE_BINARY_DIR}/test/image")
    endif()

  make_directory(${CMAKE_BINARY_DIR}/test)
  gtest_discover_tests(testexe
    EXTRA_ARGS --gtest_death_test_style=threadsafe --gtest_recreate_environments_when_repeating
    PROPERTIES
    ENVIRONMENT TEST_IMAGE_DIR=${image_dir}
    ENVIRONMENT TEST_LOG_DIR=${CMAKE_BINARY_DIR}/test
    ENVIRONMENT TEST_SRC_DIR=${CMAKE_SOURCE_DIR}/tests
    ENVIRONMENT MKE2FS=${MKE2FS}
    TIMEOUT 0)
endif()

install(TARGETS ext2_driver DESTINATION bin)
