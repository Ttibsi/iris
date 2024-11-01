include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
fetchcontent_declare(
  rawterm
  GIT_REPOSITORY https://github.com/ttibsi/rawterm
  GIT_TAG        main
)
fetchcontent_makeavailable(rawterm)

set(CPPTRACE_BUILD_SHARED Off)
set(CPPTRACE_UNWIND_WITH_UNWIND on)
fetchcontent_declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG        v0.7.0
)
fetchcontent_makeavailable(cpptrace)

include(${PROJECT_SOURCE_DIR}/cmake/version.cmake)
version_setup()

add_library(iris_src STATIC
    controller.cpp
    file_io.cpp
    model.cpp
    view.cpp
)
target_link_libraries(iris_src PUBLIC rawterm)
target_link_libraries(iris_src PUBLIC cpptrace::cpptrace)

if(ENABLE_ASAN)
    target_link_libraries(iris_src PRIVATE -fsanitize=address)
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC rawterm iris_src)
