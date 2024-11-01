function(version_setup)
    execute_process(
        COMMAND git describe
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        ERROR_VARIABLE GIT_DESC_ERROR
        RESULT_VARIABLE RET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT (RET EQUAL 0))
        execute_process(
            COMMAND git log -1 --format=%h
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            OUTPUT_VARIABLE GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        set(GIT_VERSION "v0.0.0-${GIT_HASH}")
    endif()

    string(TIMESTAMP COMPILE_DATE "%Y/%m/%d")
    if(CMAKE_BUILD_TYPE STREQUAL "")
        set(BUILD_MODE "Default")
    else()
        set(BUILD_MODE ${CMAKE_BUILD_TYPE})
    endif()

    configure_file(
        "${PROJECT_SOURCE_DIR}/src/version.h.in"
        "${PROJECT_SOURCE_DIR}/src/version.h"
        @ONLY
    )
endfunction()
