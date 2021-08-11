@echo off
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
git checkout 69478c5caafcde4c490bb1fccb960296801dbb5f
call bootstrap-vcpkg.bat
vcpkg install glfw3 glm nlohmann-json assimp stb magic-enum
rmdir /s /q .git
cd ..
pause