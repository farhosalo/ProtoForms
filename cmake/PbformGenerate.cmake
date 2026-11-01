# pbform_generate_forms(TARGET <tgt> PROTOS <a.proto ...> IMPORT_DIRS <dir ...> [OUT_DIR <dir>])
#
# Ruft protoc mit protoc-gen-qtforms auf und hängt <name>.qtform.{h,cpp}
# an das Target. PROTOS-Pfade müssen unterhalb eines IMPORT_DIRS liegen.
# Variable PBFORM_PLUGIN_EXECUTABLE überschreibt den Plugin-Pfad (für installierte Plugins).
function(pbform_generate_forms)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET;OUT_DIR" "PROTOS;IMPORT_DIRS")
    if(NOT ARG_TARGET OR NOT ARG_PROTOS OR NOT ARG_IMPORT_DIRS)
        message(FATAL_ERROR "pbform_generate_forms: TARGET, PROTOS und IMPORT_DIRS sind Pflicht")
    endif()
    if(NOT ARG_OUT_DIR)
        set(ARG_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    file(MAKE_DIRECTORY "${ARG_OUT_DIR}")

    if(PBFORM_PLUGIN_EXECUTABLE)
        set(_plugin "${PBFORM_PLUGIN_EXECUTABLE}")
        set(_plugin_dep "")
    else()
        set(_plugin "$<TARGET_FILE:protoc-gen-qtforms>")
        set(_plugin_dep protoc-gen-qtforms)
    endif()

    set(_imports "")
    foreach(_dir IN LISTS ARG_IMPORT_DIRS)
        get_filename_component(_absdir "${_dir}" ABSOLUTE)
        list(APPEND _imports "-I${_absdir}")
    endforeach()

    set(_outs "")
    foreach(_proto IN LISTS ARG_PROTOS)
        get_filename_component(_abs "${_proto}" ABSOLUTE)
        set(_rel "")
        foreach(_dir IN LISTS ARG_IMPORT_DIRS)
            get_filename_component(_absdir "${_dir}" ABSOLUTE)
            file(RELATIVE_PATH _r "${_absdir}" "${_abs}")
            if(NOT _r MATCHES "^\\.\\.")
                set(_rel "${_r}")
                break()
            endif()
        endforeach()
        if(NOT _rel)
            message(FATAL_ERROR "pbform_generate_forms: ${_proto} liegt in keinem IMPORT_DIR")
        endif()
        string(REGEX REPLACE "\\.proto$" "" _base "${_rel}")
        set(_h "${ARG_OUT_DIR}/${_base}.qtform.h")
        set(_c "${ARG_OUT_DIR}/${_base}.qtform.cpp")
        add_custom_command(
            OUTPUT "${_h}" "${_c}"
            COMMAND $<TARGET_FILE:protobuf::protoc>
                    --plugin=protoc-gen-qtforms=${_plugin}
                    --qtforms_out=${ARG_OUT_DIR}
                    ${_imports}
                    "${_abs}"
            DEPENDS "${_abs}" ${_plugin_dep}
            COMMENT "qtforms: ${_rel}"
            VERBATIM)
        list(APPEND _outs "${_h}" "${_c}")
    endforeach()

    target_sources(${ARG_TARGET} PRIVATE ${_outs})
    target_include_directories(${ARG_TARGET} PUBLIC "${ARG_OUT_DIR}")
endfunction()
