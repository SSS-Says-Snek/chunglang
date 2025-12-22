cmake_minimum_required(VERSION 3.10)
project(Chunglang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Some random ahh warning idk
if(POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
endif()

option(INSTALL_RAYLIB "Set to ON to install raylib directly from CMake" OFF)
option(IS_GITHUB_ACTIONS "Set to ON if running from GitHub Actions" OFF)

# Locate LLVM
find_package(LLVM REQUIRED CONFIG)

# Locate and Install Raylib
if(INSTALL_RAYLIB)
    include(FetchContent)
    set(FETCHCONTENT_QUIET FALSE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
    )

    FetchContent_MakeAvailable(raylib)
else()
    find_package(raylib REQUIRED)
endif()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "LLVM libraries: ${LLVM_LIBRARIES}")
message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
message(STATUS "LLVM tools: ${LLVM_TOOLS_BINARY_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(chung
    src/cli.cpp
    src/library/setup_prelude.cpp
    src/codegen.cpp
    src/context.cpp
    src/file.cpp
    src/lexer.cpp
    src/parser.cpp
    src/stringify.cpp
    src/token.cpp
    src/type.cpp
    src/sema.cpp
)
target_include_directories(chung PUBLIC include)
target_compile_options(chung PRIVATE -Wall -Wextra -Wpedantic)

# -lLLVM-20 !!!!
# llvm_map_components_to_libnames(llvm_libs support)
if(IS_GITHUB_ACTIONS)
    llvm_config(support core irreader)
else()
    llvm_config(chung -21)
endif()

target_link_libraries(chung ${llvm_libs})
