cmake_minimum_required(VERSION 3.21)
project(pbform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

# option(PBFORM_BUILD_RUNTIME  "Runtime-Bibliothek (Qt: Reflection-Form + SQLite)" OFF)
option(PBFORM_BUILD_EXAMPLES "Beispiele" OFF)
option(PBFORM_BUILD_TESTS    "Tests" OFF)

find_package(Protobuf CONFIG REQUIRED)

add_subdirectory(proto)     # Annotationen (pbform/ui_options.proto)
add_subdirectory(plugin)    # protoc-gen-qtforms  (bewusst OHNE Qt-Abhängigkeit)
include(cmake/PbformGenerate.cmake)

add_subdirectory(example1)