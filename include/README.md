### Catch2
- Clone the repo and checkout the correct git tag for the version you want to install
- Use the standard CMake command to build it
    - `cmake -S . - B build && cmake --build build`
- Install it to this repo
    - `cmake --install build --prefix ../iris/include/catch2`
