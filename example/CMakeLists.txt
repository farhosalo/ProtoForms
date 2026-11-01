find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# 1) normale protobuf-Klassen
add_library(recipes_protos STATIC recipe.proto)
target_link_libraries(recipes_protos PUBLIC pbform_options)

protobuf_generate(TARGET recipes_protos
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/proto)
target_include_directories(recipes_protos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# 2) generierte Qt-Masken (protoc-gen-qtforms)
add_library(recipes_forms STATIC)
pbform_generate_forms(TARGET recipes_forms
    PROTOS recipe.proto
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/proto)
target_link_libraries(recipes_forms PUBLIC recipes_protos Qt6::Widgets)

# 3) App
add_executable(recipes_demo main.cpp)
target_link_libraries(recipes_demo PRIVATE 
recipes_forms 
# pbform_runtime
)

set(PBFORM_RECIPES_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
