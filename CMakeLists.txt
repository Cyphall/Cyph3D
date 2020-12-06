cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")

cmake_policy(SET CMP0091 NEW)

project(Cyph3D)

add_executable(Cyph3D vendor/glad/src/glad.c vendor/imgui-1.79/include/imgui.cpp vendor/imgui-1.79/include/imgui_draw.cpp vendor/imgui-1.79/include/imgui_widgets.cpp vendor/imgui-1.79/include/imgui_stdlib.cpp  vendor/imgui-1.79/include/imgui_demo.cpp src/Main.cpp src/Engine.cpp src/Engine.h src/Logger.cpp src/Logger.h src/Window.cpp src/Window.h src/Helper/FileHelper.cpp src/Helper/FileHelper.h src/Helper/GlfwHelper.cpp src/Helper/GlfwHelper.h src/GLObject/BufferBase.cpp src/GLObject/BufferBase.h src/GLObject/ShaderProgram.cpp src/GLObject/ShaderProgram.h src/Helper/ShaderHelper.cpp src/Helper/ShaderHelper.h src/Helper/StringHelper.cpp src/Helper/StringHelper.h src/Exception/OpenGLException.h src/GLObject/Buffer.h src/GLObject/ShaderStorageBuffer.h src/GLObject/Texture.cpp src/GLObject/Texture.h src/GLObject/CreateInfo/TextureCreateInfo.h src/GLObject/Mesh.cpp src/GLObject/Mesh.h src/ResourceManagement/VertexData.h src/GLObject/VertexArray.cpp src/GLObject/VertexArray.h src/ResourceManagement/Model.cpp src/ResourceManagement/Model.h src/ResourceManagement/Resource.h src/ResourceManagement/ResourceManager.cpp src/ResourceManagement/ResourceManager.h src/ResourceManagement/Image.cpp src/ResourceManagement/Image.h src/Helper/TextureHelper.cpp src/Helper/TextureHelper.h src/GLObject/ShaderProgram_setUniform.cpp src/GLObject/Framebuffer.cpp src/GLObject/Framebuffer.h src/GLObject/Cubemap.cpp src/GLObject/Cubemap.h src/GLObject/CreateInfo/CubemapCreateInfo.h src/Helper/MathHelper.cpp src/Helper/MathHelper.h src/Scene/Transform.cpp src/Scene/Transform.h src/Helper/VectorHelper.h src/Scene/SceneObject.cpp src/Scene/SceneObject.h src/Scene/MeshObject.cpp src/Scene/MeshObject.h src/GLObject/Material.cpp src/GLObject/Material.h src/GLObject/CreateInfo/ShaderProgramCreateInfo.h src/GLObject/MaterialShaderProgram.h src/GLObject/MapDefinition.h src/GLObject/MaterialShaderProgram.cpp src/Helper/JsonHelper.cpp src/Helper/JsonHelper.h src/Scene/Light.cpp src/Scene/Light.h src/Scene/PointLight.cpp src/Scene/PointLight.h src/Scene/DirectionalLight.cpp src/Scene/DirectionalLight.h src/ResourceManagement/Skybox.cpp src/ResourceManagement/Skybox.h src/stdfloat.h src/Scene/Scene.cpp src/Scene/Scene.h src/Scene/Camera.cpp src/Scene/Camera.h src/UI/UIHelper.cpp src/UI/UIHelper.h src/UI/Impl/imgui_impl_glfw.cpp src/UI/Impl/imgui_impl_glfw.h src/UI/Impl/imgui_impl_opengl3.cpp src/UI/Impl/imgui_impl_opengl3.h src/UI/Window/UIMisc.cpp src/UI/Window/UIMisc.h src/Rendering/Renderer.cpp src/Rendering/Renderer.h src/Helper/RenderHelper.cpp src/Helper/RenderHelper.h src/GLObject/VertexBuffer.h src/Rendering/IPostProcessEffect.h src/Rendering/ToneMappingPostProcess.cpp src/Rendering/ToneMappingPostProcess.h src/UI/Window/UIHierarchy.cpp src/UI/Window/UIHierarchy.h src/Enums/ObjectType.h src/UI/Window/UIInspector.cpp src/UI/Window/UIInspector.h src/UI/Window/UIResourceExplorer.cpp src/UI/Window/UIResourceExplorer.h src/Enums/ResourceType.h src/ResourceManagement/StbImage.cpp src/ResourceManagement/StbImage.h src/Rendering/Pass/IRenderPass.h src/Rendering/Pass/GeometryPass.cpp src/Rendering/Pass/GeometryPass.h src/Rendering/Pass/ShadowMapPass.cpp src/Rendering/Pass/ShadowMapPass.h src/Rendering/SceneObjectRegistry.h src/Rendering/Pass/LightingPass.cpp src/Rendering/Pass/LightingPass.h src/Rendering/Pass/PostProcessingPass.cpp src/Rendering/Pass/PostProcessingPass.h src/Rendering/Pass/SkyboxPass.cpp src/Rendering/Pass/SkyboxPass.h src/Enums/ImageType.h)

set_property(TARGET Cyph3D PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_features(Cyph3D PRIVATE cxx_std_20)
target_compile_options(Cyph3D PRIVATE "$<$<CONFIG:Release>:-O2>")

find_path(STB_INCLUDE_DIRS "stb.h")
find_package(Assimp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_include_directories(Cyph3D PRIVATE
        "vendor/glad/include"
		"vendor/imgui-1.79/include"
		"vendor/thread_pool/include"
		"vendor/concurrentqueue/include"
		${STB_INCLUDE_DIRS})

target_link_libraries(Cyph3D PRIVATE
		assimp::assimp
		fmt::fmt
		glfw
		glm
		magic_enum::magic_enum
		nlohmann_json nlohmann_json::nlohmann_json)

file(COPY resources DESTINATION .)