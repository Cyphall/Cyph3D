# Cyph3D

This is a simple 3D engine which aims to implement standard rendering features.<br/>
This project is a full C++ rewrite of my [original C# 3D engine](https://github.com/Cyphall/Cyph3D-legacy)

## Features

### Rendering

#### Rasterization renderer

- [x] PBR Lighting
- [x] Parallax Occlusion Mapping
- [x] Skybox
- [x] Point Light
- [x] Directional Light
- [x] Point Light Shadows
- [x] Directional Light Shadows

#### Path tracing renderer

- [ ] Denoising
- [x] Viewport accumulation
- [ ] Parallax Occlusion Mapping or equivalent
- [x] Normal Maps or equivalent

#### Post-processing effects
- [x] Exposure
- [x] Bloom
- [x] AgX Tone-mapping

### UI

#### Windows
- [x] Viewport
- [x] Scene Hirarchy
- [x] Inspector
- [x] Asset Browser

#### Viewport gizmos
- [x] Transform
- [ ] Directional Light
- [ ] Point Light

## Build

Cyph3D only supports Windows. Support for any other OS is not currently planned.

### Requirements:

- Somewhat recent version of Python 3
- CMake 3.28+
- Ninja
- Latest MSVC (with the English Language Pack, vcpkg won't work otherwise, see [microsoft/vcpkg#3842](https://github.com/microsoft/vcpkg/issues/3842))

### Steps:

```bash
cmake -B bin -G Ninja -D CMAKE_BUILD_TYPE=Release
cmake --build bin
```

## Screenshots

![](screenshots/01.jpg?raw=true "Cyph3D Interface")
![](screenshots/02.jpg?raw=true "Dungeon Scene")
![](screenshots/03.jpg?raw=true "Spaceship Scene")
![](screenshots/04.jpg?raw=true "Alien Alter Scene")
![](screenshots/05.jpg?raw=true "Egyptian Temple Scene")
![](screenshots/06.jpg?raw=true "Path tracing demo Scene 1")
![](screenshots/07.jpg?raw=true "Path tracing demo Scene 2")

## License

This work by [Cyphall](https://github.com/Cyphall) is licensed under the [Creative Commons Attribution Non Commercial Share Alike 4.0 International](https://spdx.org/licenses/CC-BY-NC-SA-4.0.html).

## Third party licenses

[Font Awesome Free](https://fontawesome.com/license/free) is licensed under the [SIL Open Font License 1.1](https://spdx.org/licenses/OFL-1.1.html).<br/>
[bshoshany/thread-pool](https://github.com/bshoshany/thread-pool) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[ocornut/imgui](https://github.com/ocornut/imgui) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[CedricGuillemet/ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[palacaze/sigslot](https://github.com/palacaze/sigslot) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[YaaZ/VulkanMemoryAllocator-Hpp](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp) is licensed under the [Creative Commons Zero v1.0 Universal](https://spdx.org/licenses/CC0-1.0.html).<br/>
[GLFW](https://www.glfw.org/) is licensed under the [zlib License](https://spdx.org/licenses/Zlib.html).<br/>
[g-truc/glm](https://github.com/g-truc/glm) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[nlohmann/json](https://github.com/nlohmann/json) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[assimp/assimp](https://github.com/assimp/assimp) is licensed under [their own license](https://github.com/assimp/assimp/blob/master/LICENSE).<br/>
[nothings/stb](https://github.com/nothings/stb) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[Neargye/magic_enum](https://github.com/Neargye/magic_enum) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[FreeType](https://freetype.org) is licensed under the [Freetype Project License](https://spdx.org/licenses/FTL.html).<br/>
[SRombauts/SQLiteCpp](https://github.com/SRombauts/SQLiteCpp) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[graeme-hill/crossguid](https://github.com/graeme-hill/crossguid) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[GameTechDev/ISPCTextureCompressor](https://github.com/GameTechDev/ISPCTextureCompressor) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).<br/>
[half](https://half.sourceforge.net/) is licensed under the [MIT License](https://spdx.org/licenses/MIT.html).
