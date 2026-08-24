include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
set(RAWTERM_GIT_TAG "main")
fetchcontent_declare(
  rawterm
  GIT_REPOSITORY https://github.com/ttibsi/rawterm
  GIT_TAG        ${RAWTERM_GIT_TAG}
)
fetchcontent_makeavailable(rawterm)

include(${PROJECT_SOURCE_DIR}/cmake/version.cmake)
version_setup()

add_library(iris_src STATIC
    controller.cpp
    model.cpp
    text_io.cpp
    view.cpp
)
target_link_libraries(iris_src PUBLIC rawterm)

if(RELEASE)
    set_property(TARGET iris_src PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(ENABLE_ASAN)
    target_link_libraries(iris_src PRIVATE -fsanitize=address)
endif()

if(ENABLE_PROFILING)
    add_subdirectory(
        ${PROJECT_SOURCE_DIR}/include/tracy
        ${CMAKE_BINARY_DIR}/tracy
    )

    target_link_libraries(iris_src PRIVATE TracyClient)
    target_compile_definitions(iris_src PRIVATE TRACY_ENABLE)
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC rawterm iris_src)
target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)
