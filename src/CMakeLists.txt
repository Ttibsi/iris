include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
fetchcontent_declare(
  rawterm
  GIT_REPOSITORY https://github.com/ttibsi/rawterm
  GIT_TAG        main
)
fetchcontent_makeavailable(rawterm)

include(${PROJECT_SOURCE_DIR}/cmake/version.cmake)
version_setup()

# add_library(iris_src STATIC
# )
# target_link_libraries(iris_src PUBLIC rawterm)

if(ENABLE_ASAN)
    target_link_libraries(iris_src PRIVATE -fsanitize=address)
endif()

add_executable(${PROJECT_NAME} main.cpp)
# target_link_libraries(${PROJECT_NAME} PUBLIC rawterm iris_src)
target_link_libraries(${PROJECT_NAME} PUBLIC rawterm)
