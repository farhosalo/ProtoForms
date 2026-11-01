# Der Generator linkt absichtlich NICHT gegen Qt (nur protobuf + libprotoc).
add_executable(protoc-gen-qtforms
    main.cpp
    QtFormsGenerator.cpp
    QtFormsGenerator.h)
target_link_libraries(protoc-gen-qtforms PRIVATE
    pbform_options
    protobuf::libprotoc
    protobuf::libprotobuf)

install(TARGETS protoc-gen-qtforms RUNTIME DESTINATION bin)
