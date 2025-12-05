cmake_minimum_required(VERSION 3.10)
project(Chunglang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Some random ahh warning idk
if(POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
endif()

# Locate LLVM
find_package(LLVM REQUIRED CONFIG)

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
    # src/codegen.cpp
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
# llvm_map_components_to_libnames(llvm_libs -20)
llvm_config(chung -21)

target_link_libraries(chung ${llvm_libs})
