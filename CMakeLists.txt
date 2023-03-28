cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

cmake_policy(SET CMP0091 NEW)

project(Cyph3D)

add_executable(Cyph3D src/Cyph3D/Main.cpp src/Cyph3D/Engine.cpp src/Cyph3D/Engine.h src/Cyph3D/Logging/Logger.cpp src/Cyph3D/Logging/Logger.h src/Cyph3D/Window.cpp src/Cyph3D/Window.h src/Cyph3D/Helper/FileHelper.cpp src/Cyph3D/Helper/FileHelper.h src/Cyph3D/Helper/GlfwHelper.cpp src/Cyph3D/Helper/GlfwHelper.h src/Cyph3D/GLObject/GLShaderProgram.cpp src/Cyph3D/GLObject/GLShaderProgram.h src/Cyph3D/GLObject/GLShader.cpp src/Cyph3D/GLObject/GLShader.h src/Cyph3D/Helper/StringHelper.cpp src/Cyph3D/Helper/StringHelper.h src/Cyph3D/Exception/OpenGLException.h src/Cyph3D/GLObject/GLTexture.cpp src/Cyph3D/GLObject/GLTexture.h src/Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h src/Cyph3D/GLObject/Mesh.cpp src/Cyph3D/GLObject/Mesh.h src/Cyph3D/Helper/TextureHelper.cpp src/Cyph3D/Helper/TextureHelper.h src/Cyph3D/GLObject/GLShaderProgram_setUniform.cpp src/Cyph3D/GLObject/GLFramebuffer.cpp src/Cyph3D/GLObject/GLFramebuffer.h src/Cyph3D/GLObject/GLCubemap.cpp src/Cyph3D/GLObject/GLCubemap.h src/Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h src/Cyph3D/Helper/MathHelper.cpp src/Cyph3D/Helper/MathHelper.h src/Cyph3D/Scene/Transform.cpp src/Cyph3D/Scene/Transform.h src/Cyph3D/Helper/VectorHelper.h src/Cyph3D/Helper/JsonHelper.cpp src/Cyph3D/Helper/JsonHelper.h src/Cyph3D/Scene/Scene.cpp src/Cyph3D/Scene/Scene.h src/Cyph3D/Scene/Camera.cpp src/Cyph3D/Scene/Camera.h src/Cyph3D/UI/UIHelper.cpp src/Cyph3D/UI/UIHelper.h src/Cyph3D/UI/Window/UIMisc.cpp src/Cyph3D/UI/Window/UIMisc.h src/Cyph3D/Rendering/Renderer/RasterizationRenderer.cpp src/Cyph3D/Rendering/Renderer/RasterizationRenderer.h src/Cyph3D/Helper/RenderHelper.cpp src/Cyph3D/Helper/RenderHelper.h src/Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h src/Cyph3D/Rendering/PostProcessingEffect/ToneMappingEffect.cpp src/Cyph3D/Rendering/PostProcessingEffect/ToneMappingEffect.h src/Cyph3D/UI/Window/UIHierarchy.cpp src/Cyph3D/UI/Window/UIHierarchy.h src/Cyph3D/UI/Window/UIInspector.cpp src/Cyph3D/UI/Window/UIInspector.h src/Cyph3D/UI/Window/UIResourceExplorer.cpp src/Cyph3D/UI/Window/UIResourceExplorer.h src/Cyph3D/Enums/ResourceType.h src/Cyph3D/StbImage.cpp src/Cyph3D/StbImage.h src/Cyph3D/Rendering/Pass/RenderPass.cpp src/Cyph3D/Rendering/Pass/RenderPass.h src/Cyph3D/Rendering/Pass/ShadowMapPass.cpp src/Cyph3D/Rendering/Pass/ShadowMapPass.h src/Cyph3D/Rendering/RenderRegistry.h src/Cyph3D/Rendering/Pass/PostProcessingPass.cpp src/Cyph3D/Rendering/Pass/PostProcessingPass.h src/Cyph3D/Rendering/Pass/SkyboxPass.cpp src/Cyph3D/Rendering/Pass/SkyboxPass.h src/Cyph3D/Rendering/Pass/ZPrePass.cpp src/Cyph3D/Rendering/Pass/ZPrePass.h src/Cyph3D/Timer.cpp src/Cyph3D/Timer.h src/Cyph3D/GLObject/CreateInfo/SamplerCreateInfo.h src/Cyph3D/GLObject/GLSampler.cpp src/Cyph3D/GLObject/GLSampler.h src/Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.cpp src/Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.h src/Cyph3D/Rendering/PostProcessingEffect/BloomEffect.cpp src/Cyph3D/Rendering/PostProcessingEffect/BloomEffect.h src/Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.cpp src/Cyph3D/Entity/Entity.cpp src/Cyph3D/Entity/Entity.h src/Cyph3D/Entity/Component/Component.h src/Cyph3D/Iterator/ComponentIterator.h src/Cyph3D/Iterator/ComponentConstIterator.h src/Cyph3D/Entity/Component/Component.cpp src/Cyph3D/Entity/Component/ShapeRenderer.cpp src/Cyph3D/Entity/Component/ShapeRenderer.h src/Cyph3D/Entity/Component/Animator.cpp src/Cyph3D/Entity/Component/Animator.h src/Cyph3D/Entity/Component/LightBase.cpp src/Cyph3D/Entity/Component/LightBase.h src/Cyph3D/Entity/Component/PointLight.cpp src/Cyph3D/Entity/Component/PointLight.h src/Cyph3D/Entity/Component/DirectionalLight.cpp src/Cyph3D/Entity/Component/DirectionalLight.h src/Cyph3D/Iterator/ComponentIterator.cpp src/Cyph3D/Iterator/ComponentConstIterator.cpp src/Cyph3D/Iterator/EntityConstIterator.cpp src/Cyph3D/Iterator/EntityConstIterator.h src/Cyph3D/Iterator/EntityIterator.cpp src/Cyph3D/Iterator/EntityIterator.h src/Cyph3D/Event.h src/Cyph3D/UI/Window/UIMenuBar.cpp src/Cyph3D/UI/Window/UIMenuBar.h src/Cyph3D/UI/Window/UIViewport.cpp src/Cyph3D/UI/Window/UIViewport.h src/Cyph3D/RenderContext.h src/Cyph3D/Helper/ThreadHelper.cpp src/Cyph3D/Helper/ThreadHelper.h src/Cyph3D/ObjectSerialization.h src/Cyph3D/ObjectSerialization.cpp  src/Cyph3D/Rendering/Shape/Shape.cpp src/Cyph3D/Rendering/Shape/Shape.h src/Cyph3D/Rendering/Shape/MeshShape.cpp src/Cyph3D/Rendering/Shape/MeshShape.h src/Cyph3D/Rendering/Shape/SphereShape.cpp src/Cyph3D/Rendering/Shape/SphereShape.h src/Cyph3D/Rendering/Renderer/RaytracingRenderer.cpp src/Cyph3D/Rendering/Renderer/RaytracingRenderer.h src/Cyph3D/Rendering/Renderer/Renderer.cpp src/Cyph3D/Rendering/Renderer/Renderer.h src/Cyph3D/Rendering/Pass/RaytracePass.cpp src/Cyph3D/Rendering/Pass/RaytracePass.h src/Cyph3D/Rendering/Shape/PlaneShape.cpp src/Cyph3D/Rendering/Shape/PlaneShape.h src/Cyph3D/PerfCounter/PerfStep.h src/Cyph3D/PerfCounter/GpuPerfCounter.cpp src/Cyph3D/PerfCounter/GpuPerfCounter.h src/Cyph3D/Logging/Impl/ILoggerColor.h src/Cyph3D/Logging/Impl/Win32LoggerColor.cpp src/Cyph3D/Logging/Impl/Win32LoggerColor.h src/Cyph3D/Logging/LogColorFlags.h src/Cyph3D/GLSL_types.h src/Cyph3D/LibImpl.cpp src/Cyph3D/GLObject/GLObject.cpp src/Cyph3D/GLObject/GLObject.h src/Cyph3D/GLObject/GLBuffer.h src/Cyph3D/GLObject/GLMutableBuffer.h src/Cyph3D/GLObject/GLImmutableBuffer.h src/Cyph3D/GLObject/GLVertexArray.cpp src/Cyph3D/GLObject/GLVertexArray.h src/Cyph3D/GLObject/GLFence.cpp src/Cyph3D/GLObject/GLFence.h src/Cyph3D/UI/IInspectable.h src/Cyph3D/UI/Window/UIAssetBrowser.cpp src/Cyph3D/UI/Window/UIAssetBrowser.h src/Cyph3D/Asset/RuntimeAsset/TextureAsset.cpp src/Cyph3D/Asset/RuntimeAsset/TextureAsset.h src/Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h src/Cyph3D/Asset/AssetCacheDatabase.cpp src/Cyph3D/Asset/AssetCacheDatabase.h src/Cyph3D/Asset/AssetManager.cpp src/Cyph3D/Asset/AssetManager.h src/Cyph3D/Enums/TextureType.h src/Cyph3D/Asset/Processor/ImageProcessor.cpp src/Cyph3D/Asset/Processor/ImageProcessor.h src/Cyph3D/Asset/Processor/MeshProcessor.cpp src/Cyph3D/Asset/Processor/MeshProcessor.h src/Cyph3D/Asset/Processor/ImageData.h src/Cyph3D/Asset/Processor/MeshData.h src/Cyph3D/HashBuilder.h src/Cyph3D/Asset/RuntimeAsset/CubemapAsset.cpp src/Cyph3D/Asset/RuntimeAsset/CubemapAsset.h src/Cyph3D/Asset/RuntimeAsset/ModelAsset.cpp src/Cyph3D/Asset/RuntimeAsset/ModelAsset.h src/Cyph3D/Asset/RuntimeAsset/MaterialAsset.cpp src/Cyph3D/Asset/RuntimeAsset/MaterialAsset.h src/Cyph3D/Asset/RuntimeAsset/GPUAsset.h src/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.cpp src/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h src/Cyph3D/Helper/ImGuiHelper.cpp src/Cyph3D/Helper/ImGuiHelper.h src/Cyph3D/Rendering/Pass/LightingPass.cpp src/Cyph3D/Rendering/Pass/LightingPass.h)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # MSVC
	target_compile_options(Cyph3D PRIVATE /Zc:__cplusplus)
	target_compile_options(Cyph3D PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") # clang-cl
	target_compile_options(Cyph3D PRIVATE /Zc:__cplusplus)
	target_compile_options(Cyph3D PRIVATE /utf-8)
	target_compile_options(Cyph3D PRIVATE /EHsc)
endif()

target_compile_features(Cyph3D PRIVATE cxx_std_20)

target_include_directories(Cyph3D PRIVATE "src")

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
target_include_directories(Cyph3D PRIVATE ${STB_INCLUDE_DIRS})

find_package(assimp CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE assimp::assimp)

find_package(freetype CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE freetype)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE glm::glm)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE magic_enum::magic_enum)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE nlohmann_json::nlohmann_json)

find_package(SQLiteCpp CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE SQLiteCpp)

find_package(crossguid CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE crossguid)

add_subdirectory("vendor/bs-thread-pool/v3.3.0")
target_link_libraries(Cyph3D PRIVATE bs-thread-pool)

add_subdirectory("vendor/glad/generated")
target_link_libraries(Cyph3D PRIVATE glad)

add_subdirectory("vendor/imgui/v1.89.4-docking")
target_link_libraries(Cyph3D PRIVATE imgui)

add_subdirectory("vendor/imguizmo/fbc3614abcc642de7f3868cc0d68a35503ca9a4e")
target_link_libraries(Cyph3D PRIVATE imguizmo)

add_custom_target(copy_resources
		COMMAND robocopy ${PROJECT_SOURCE_DIR}/resources/ ${PROJECT_BINARY_DIR}/resources /E /NS /NC /NFL /NDL /NP /NJH /NJS || cmd /c "exit /b 0")
add_dependencies(Cyph3D copy_resources)