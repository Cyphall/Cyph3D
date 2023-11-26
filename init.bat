@echo off
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
git checkout cd5e746ec203c8c3c61647e0886a8df8c1e78e41
call bootstrap-vcpkg.bat
vcpkg install glfw3 glm nlohmann-json assimp stb magic-enum freetype sqlitecpp crossguid
rmdir /s /q .git
rmdir /s /q buildtrees
rmdir /s /q downloads
rmdir /s /q packages
cd ..
pause