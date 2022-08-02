cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")

cmake_policy(SET CMP0091 NEW)

project(Cyph3D)

add_executable(Cyph3D vendor/imguizmo-8afa1d7050392111cb917a6439dc76450cda5aa6/src/ImGuizmo.cpp vendor/glad/src/glad.c vendor/imgui/src/imgui.cpp vendor/imgui/src/imgui_demo.cpp vendor/imgui/src/imgui_draw.cpp vendor/imgui/src/imgui_impl_glfw.cpp vendor/imgui/src/imgui_impl_opengl3.cpp vendor/imgui/src/imgui_stdlib.cpp vendor/imgui/src/imgui_tables.cpp vendor/imgui/src/imgui_widgets.cpp src/Cyph3D/Main.cpp src/Cyph3D/Engine.cpp src/Cyph3D/Engine.h src/Cyph3D/Logging/Logger.cpp src/Cyph3D/Logging/Logger.h src/Cyph3D/Window.cpp src/Cyph3D/Window.h src/Cyph3D/Helper/FileHelper.cpp src/Cyph3D/Helper/FileHelper.h src/Cyph3D/Helper/GlfwHelper.cpp src/Cyph3D/Helper/GlfwHelper.h src/Cyph3D/GLObject/GLShaderProgram.cpp src/Cyph3D/GLObject/GLShaderProgram.h src/Cyph3D/Helper/ShaderHelper.cpp src/Cyph3D/Helper/ShaderHelper.h src/Cyph3D/Helper/StringHelper.cpp src/Cyph3D/Helper/StringHelper.h src/Cyph3D/Exception/OpenGLException.h src/Cyph3D/GLObject/GLTexture.cpp src/Cyph3D/GLObject/GLTexture.h src/Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h src/Cyph3D/GLObject/Mesh.cpp src/Cyph3D/GLObject/Mesh.h src/Cyph3D/ResourceManagement/Model.cpp src/Cyph3D/ResourceManagement/Model.h src/Cyph3D/ResourceManagement/Resource.h src/Cyph3D/ResourceManagement/ResourceManager.cpp src/Cyph3D/ResourceManagement/ResourceManager.h src/Cyph3D/ResourceManagement/Image.cpp src/Cyph3D/ResourceManagement/Image.h src/Cyph3D/Helper/TextureHelper.cpp src/Cyph3D/Helper/TextureHelper.h src/Cyph3D/GLObject/GLShaderProgram_setUniform.cpp src/Cyph3D/GLObject/GLFramebuffer.cpp src/Cyph3D/GLObject/GLFramebuffer.h src/Cyph3D/GLObject/GLCubemap.cpp src/Cyph3D/GLObject/GLCubemap.h src/Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h src/Cyph3D/Helper/MathHelper.cpp src/Cyph3D/Helper/MathHelper.h src/Cyph3D/Scene/Transform.cpp src/Cyph3D/Scene/Transform.h src/Cyph3D/Helper/VectorHelper.h src/Cyph3D/GLObject/Material/Material.cpp src/Cyph3D/GLObject/Material/Material.h src/Cyph3D/GLObject/CreateInfo/ShaderProgramCreateInfo.h src/Cyph3D/GLObject/Material/MaterialMapDefinition.h src/Cyph3D/Helper/JsonHelper.cpp src/Cyph3D/Helper/JsonHelper.h src/Cyph3D/ResourceManagement/Skybox.cpp src/Cyph3D/ResourceManagement/Skybox.h src/Cyph3D/Scene/Scene.cpp src/Cyph3D/Scene/Scene.h src/Cyph3D/Scene/Camera.cpp src/Cyph3D/Scene/Camera.h src/Cyph3D/UI/UIHelper.cpp src/Cyph3D/UI/UIHelper.h src/Cyph3D/UI/Window/UIMisc.cpp src/Cyph3D/UI/Window/UIMisc.h src/Cyph3D/Rendering/Renderer/RasterizationRenderer.cpp src/Cyph3D/Rendering/Renderer/RasterizationRenderer.h src/Cyph3D/Helper/RenderHelper.cpp src/Cyph3D/Helper/RenderHelper.h src/Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h src/Cyph3D/Rendering/PostProcessingEffect/ToneMappingEffect.cpp src/Cyph3D/Rendering/PostProcessingEffect/ToneMappingEffect.h src/Cyph3D/UI/Window/UIHierarchy.cpp src/Cyph3D/UI/Window/UIHierarchy.h src/Cyph3D/UI/Window/UIInspector.cpp src/Cyph3D/UI/Window/UIInspector.h src/Cyph3D/UI/Window/UIResourceExplorer.cpp src/Cyph3D/UI/Window/UIResourceExplorer.h src/Cyph3D/Enums/ResourceType.h src/Cyph3D/ResourceManagement/StbImage.cpp src/Cyph3D/ResourceManagement/StbImage.h src/Cyph3D/Rendering/Pass/RenderPass.cpp src/Cyph3D/Rendering/Pass/RenderPass.h src/Cyph3D/Rendering/Pass/GeometryPass.cpp src/Cyph3D/Rendering/Pass/GeometryPass.h src/Cyph3D/Rendering/Pass/ShadowMapPass.cpp src/Cyph3D/Rendering/Pass/ShadowMapPass.h src/Cyph3D/Rendering/RenderRegistry.h src/Cyph3D/Rendering/Pass/LightingPass.cpp src/Cyph3D/Rendering/Pass/LightingPass.h src/Cyph3D/Rendering/Pass/PostProcessingPass.cpp src/Cyph3D/Rendering/Pass/PostProcessingPass.h src/Cyph3D/Rendering/Pass/SkyboxPass.cpp src/Cyph3D/Rendering/Pass/SkyboxPass.h src/Cyph3D/Enums/ImageType.h src/Cyph3D/Rendering/Pass/ZPrePass.cpp src/Cyph3D/Rendering/Pass/ZPrePass.h src/Cyph3D/Rendering/Pass/GBufferDebugPass.cpp src/Cyph3D/Rendering/Pass/GBufferDebugPass.h src/Cyph3D/Timer.cpp src/Cyph3D/Timer.h src/Cyph3D/GLObject/CreateInfo/SamplerCreateInfo.h src/Cyph3D/GLObject/GLSampler.cpp src/Cyph3D/GLObject/GLSampler.h src/Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.cpp src/Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.h src/Cyph3D/Rendering/PostProcessingEffect/BloomEffect.cpp src/Cyph3D/Rendering/PostProcessingEffect/BloomEffect.h src/Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.cpp src/Cyph3D/Entity/Entity.cpp src/Cyph3D/Entity/Entity.h src/Cyph3D/Entity/Component/Component.h src/Cyph3D/Iterator/ComponentIterator.h src/Cyph3D/Iterator/ComponentConstIterator.h src/Cyph3D/Entity/Component/Component.cpp src/Cyph3D/Entity/Component/ShapeRenderer.cpp src/Cyph3D/Entity/Component/ShapeRenderer.h src/Cyph3D/Entity/Component/Animator.cpp src/Cyph3D/Entity/Component/Animator.h src/Cyph3D/Entity/Component/LightBase.cpp src/Cyph3D/Entity/Component/LightBase.h src/Cyph3D/Entity/Component/PointLight.cpp src/Cyph3D/Entity/Component/PointLight.h src/Cyph3D/Entity/Component/DirectionalLight.cpp src/Cyph3D/Entity/Component/DirectionalLight.h src/Cyph3D/Iterator/ComponentIterator.cpp src/Cyph3D/Iterator/ComponentConstIterator.cpp src/Cyph3D/Iterator/EntityConstIterator.cpp src/Cyph3D/Iterator/EntityConstIterator.h src/Cyph3D/Iterator/EntityIterator.cpp src/Cyph3D/Iterator/EntityIterator.h src/Cyph3D/Event.h src/Cyph3D/UI/Window/UIMenuBar.cpp src/Cyph3D/UI/Window/UIMenuBar.h src/Cyph3D/UI/Window/UIViewport.cpp src/Cyph3D/UI/Window/UIViewport.h src/Cyph3D/RenderContext.h src/Cyph3D/Helper/ThreadHelper.cpp src/Cyph3D/Helper/ThreadHelper.h src/Cyph3D/ObjectSerialization.h src/Cyph3D/ObjectSerialization.cpp  src/Cyph3D/Rendering/Shape/Shape.cpp src/Cyph3D/Rendering/Shape/Shape.h src/Cyph3D/Rendering/Shape/MeshShape.cpp src/Cyph3D/Rendering/Shape/MeshShape.h src/Cyph3D/Rendering/Shape/SphereShape.cpp src/Cyph3D/Rendering/Shape/SphereShape.h src/Cyph3D/Rendering/Renderer/RaytracingRenderer.cpp src/Cyph3D/Rendering/Renderer/RaytracingRenderer.h src/Cyph3D/Rendering/Renderer/Renderer.cpp src/Cyph3D/Rendering/Renderer/Renderer.h src/Cyph3D/Rendering/Pass/RaytracePass.cpp src/Cyph3D/Rendering/Pass/RaytracePass.h src/Cyph3D/Rendering/Shape/PlaneShape.cpp src/Cyph3D/Rendering/Shape/PlaneShape.h src/Cyph3D/GLObject/Material/MaterialMap.h src/Cyph3D/GLObject/Material/MaterialMapType.h src/Cyph3D/PerfCounter/PerfStep.h src/Cyph3D/PerfCounter/GpuPerfCounter.cpp src/Cyph3D/PerfCounter/GpuPerfCounter.h src/Cyph3D/Logging/Impl/ILoggerColor.h src/Cyph3D/Logging/Impl/Win32LoggerColor.cpp src/Cyph3D/Logging/Impl/Win32LoggerColor.h src/Cyph3D/Logging/LogColorFlags.h src/Cyph3D/GLSL_types.h src/Cyph3D/LibImpl.cpp src/Cyph3D/GLObject/GLObject.cpp src/Cyph3D/GLObject/GLObject.h src/Cyph3D/GLObject/GLBuffer.h src/Cyph3D/GLObject/GLMutableBuffer.h src/Cyph3D/GLObject/GLImmutableBuffer.h src/Cyph3D/GLObject/GLVertexArray.cpp src/Cyph3D/GLObject/GLVertexArray.h src/Cyph3D/GLObject/GLFence.cpp src/Cyph3D/GLObject/GLFence.h)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # MSVC
	set_property(TARGET Cyph3D PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") # clang-cl
	set_property(TARGET Cyph3D PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	target_compile_options(Cyph3D PRIVATE /EHsc)
endif()

target_compile_features(Cyph3D PRIVATE cxx_std_20)

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
target_include_directories(Cyph3D PRIVATE ${STB_INCLUDE_DIRS})

find_package(assimp CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE assimp::assimp)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE glm::glm)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE magic_enum::magic_enum)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE nlohmann_json::nlohmann_json)

target_include_directories(Cyph3D PRIVATE "src")
target_include_directories(Cyph3D SYSTEM PRIVATE "vendor/glad/include")
target_include_directories(Cyph3D SYSTEM PRIVATE "vendor/imgui/include")
target_include_directories(Cyph3D SYSTEM PRIVATE "vendor/thread-pool/include")
target_include_directories(Cyph3D SYSTEM PRIVATE "vendor/concurrentqueue/include")
target_include_directories(Cyph3D SYSTEM PRIVATE "vendor/imguizmo-8afa1d7050392111cb917a6439dc76450cda5aa6/include")

add_custom_command(TARGET Cyph3D POST_BUILD
		COMMAND robocopy ${PROJECT_SOURCE_DIR}/resources/ ${PROJECT_BINARY_DIR}/resources /E /NS /NC /NFL /NDL /NP /NJH /NJS  || cmd /c "exit /b 0")