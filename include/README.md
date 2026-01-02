### Catch2
- Clone the repo and checkout the correct git tag for the version you want to install
- Use the standard CMake command to build it
    - `cmake -S . - B build && cmake --build build`
- Install it to this repo
    - `cmake --install build --prefix ../iris/include/catch2`

### SPDLOG
- Clone the git repo: `git@github.com:gabime/spdlog.git` and checkout the git tag
    - Currently, git commit `32dd298d` is used as the latest tag (v1.16.0) does
    not work with clang21
- copy `spdlog/include/spdlog` to `iris/include/spdlog`

