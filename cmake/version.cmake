function(version_setup)
    execute_process(
        COMMAND git describe --tags
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT GIT_TAG OR NOT GIT_HASH)
        set(GIT_VERSION "Unknown")
    else()
        set(GIT_VERSION "${GIT_TAG} (${GIT_HASH})")
    endif()

    string(TIMESTAMP COMPILE_DATE "%Y/%m/%d")
    if(CMAKE_BUILD_TYPE STREQUAL "")
        set(BUILD_MODE "Default")
    else()
        set(BUILD_MODE ${CMAKE_BUILD_TYPE})
    endif()

    set(RAWTERM_GIT_TAG "v4.0.9")
    # Needed to propagate value up to src/CMakeLists.txt
    set(RAWTERM_GIT_TAG ${RAWTERM_GIT_TAG} PARENT_SCOPE)

    configure_file(
        "${PROJECT_SOURCE_DIR}/src/version.h.in"
        "${PROJECT_SOURCE_DIR}/src/version.h"
        @ONLY
    )
endfunction()
