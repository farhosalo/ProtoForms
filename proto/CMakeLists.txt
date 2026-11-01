add_library(pbform_options STATIC pbform/ui_options.proto)
target_link_libraries(pbform_options PUBLIC protobuf::libprotobuf)
protobuf_generate(TARGET pbform_options
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(pbform_options PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
