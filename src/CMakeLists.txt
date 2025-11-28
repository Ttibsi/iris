include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
fetchcontent_declare(
  rawterm
  GIT_REPOSITORY https://github.com/ttibsi/rawterm
  GIT_TAG        b7d0f399b236b069d99ee41382a972a2b6f63607
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

if(ENABLE_ASAN)
    target_link_libraries(iris_src PRIVATE -fsanitize=address)
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC rawterm iris_src)
