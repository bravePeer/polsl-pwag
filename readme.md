1. Install C++ compiler using MSYS2
    - in UCRT64 console `pacman -S mingw-w64-ucrt-x86_64-gcc`
2. Install VulkanSDK
3. Install vcpkg and following packages
    - `vcpkg install glfw3`
    - `vcpkg install glm`
    - `vcpkg install shader-slang`
4. Install Ninja
5. Install CMake
6. To configure project run:
    `cmake -B build -G Ninja "-DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake"`