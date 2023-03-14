@echo off
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
git checkout cfdeb75bb67fee475b517a2f45d8f93f2b46bdb4
call bootstrap-vcpkg.bat
vcpkg install glfw3 glm nlohmann-json assimp stb magic-enum freetype sqlitecpp crossguid
rmdir /s /q .git
rmdir /s /q packages
rmdir /s /q buildtrees
rmdir /s /q downloads
cd ..
pause