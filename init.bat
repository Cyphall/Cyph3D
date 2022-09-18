@echo off
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
git checkout 01b29f6d8212bc845da64773b18665d682f5ab66
call bootstrap-vcpkg.bat
vcpkg install glfw3 glm nlohmann-json assimp stb magic-enum freetype
rmdir /s /q .git
cd ..
pause