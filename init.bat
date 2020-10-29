@echo off
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
call bootstrap-vcpkg.bat
vcpkg install glfw3 glm nlohmann-json assimp stb fmt magic-enum --triplet x64-windows-static
pause