cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")

cmake_policy(SET CMP0091 NEW)

project(Cyph3D)

add_executable(Cyph3D vendor/imguizmo-f7bbbe3/src/ImCurveEdit.cpp vendor/imguizmo-f7bbbe3/src/ImGradient.cpp vendor/imguizmo-f7bbbe3/src/ImGuizmo.cpp vendor/imguizmo-f7bbbe3/src/ImSequencer.cpp vendor/glad/src/glad.c vendor/imgui/include/imgui.cpp vendor/imgui/include/imgui_demo.cpp vendor/imgui/include/imgui_draw.cpp vendor/imgui/include/imgui_stdlib.cpp vendor/imgui/include/imgui_tables.cpp vendor/imgui/include/imgui_widgets.cpp src/Main.cpp src/Engine.cpp src/Engine.h src/Logger.cpp src/Logger.h src/Window.cpp src/Window.h src/Helper/FileHelper.cpp src/Helper/FileHelper.h src/Helper/GlfwHelper.cpp src/Helper/GlfwHelper.h src/GLObject/BufferBase.cpp src/GLObject/BufferBase.h src/GLObject/ShaderProgram.cpp src/GLObject/ShaderProgram.h src/Helper/ShaderHelper.cpp src/Helper/ShaderHelper.h src/Helper/StringHelper.cpp src/Helper/StringHelper.h src/Exception/OpenGLException.h src/GLObject/Buffer.h src/GLObject/ShaderStorageBuffer.h src/GLObject/Texture.cpp src/GLObject/Texture.h src/GLObject/CreateInfo/TextureCreateInfo.h src/GLObject/Mesh.cpp src/GLObject/Mesh.h src/GLObject/VertexArray.h src/ResourceManagement/Model.cpp src/ResourceManagement/Model.h src/ResourceManagement/Resource.h src/ResourceManagement/ResourceManager.cpp src/ResourceManagement/ResourceManager.h src/ResourceManagement/Image.cpp src/ResourceManagement/Image.h src/Helper/TextureHelper.cpp src/Helper/TextureHelper.h src/GLObject/ShaderProgram_setUniform.cpp src/GLObject/Framebuffer.cpp src/GLObject/Framebuffer.h src/GLObject/Cubemap.cpp src/GLObject/Cubemap.h src/GLObject/CreateInfo/CubemapCreateInfo.h src/Helper/MathHelper.cpp src/Helper/MathHelper.h src/Scene/Transform.cpp src/Scene/Transform.h src/Helper/VectorHelper.h src/GLObject/Material.cpp src/GLObject/Material.h src/GLObject/CreateInfo/ShaderProgramCreateInfo.h src/GLObject/MaterialShaderProgram.h src/GLObject/MapDefinition.h src/GLObject/MaterialShaderProgram.cpp src/Helper/JsonHelper.cpp src/Helper/JsonHelper.h src/ResourceManagement/Skybox.cpp src/ResourceManagement/Skybox.h src/stdfloat.h src/Scene/Scene.cpp src/Scene/Scene.h src/Scene/Camera.cpp src/Scene/Camera.h src/UI/UIHelper.cpp src/UI/UIHelper.h src/UI/Impl/imgui_impl_glfw.cpp src/UI/Impl/imgui_impl_glfw.h src/UI/Impl/imgui_impl_opengl3.cpp src/UI/Impl/imgui_impl_opengl3.h src/UI/Window/UIMisc.cpp src/UI/Window/UIMisc.h src/Rendering/Renderer.cpp src/Rendering/Renderer.h src/Helper/RenderHelper.cpp src/Helper/RenderHelper.h src/Rendering/PostProcessingEffect/PostProcessingEffect.h src/Rendering/PostProcessingEffect/ToneMappingEffect.cpp src/Rendering/PostProcessingEffect/ToneMappingEffect.h src/UI/Window/UIHierarchy.cpp src/UI/Window/UIHierarchy.h src/UI/Window/UIInspector.cpp src/UI/Window/UIInspector.h src/UI/Window/UIResourceExplorer.cpp src/UI/Window/UIResourceExplorer.h src/Enums/ResourceType.h src/ResourceManagement/StbImage.cpp src/ResourceManagement/StbImage.h src/Rendering/Pass/RenderPass.cpp src/Rendering/Pass/RenderPass.h src/Rendering/Pass/GeometryPass.cpp src/Rendering/Pass/GeometryPass.h src/Rendering/Pass/ShadowMapPass.cpp src/Rendering/Pass/ShadowMapPass.h src/Rendering/RenderRegistry.h src/Rendering/Pass/LightingPass.cpp src/Rendering/Pass/LightingPass.h src/Rendering/Pass/PostProcessingPass.cpp src/Rendering/Pass/PostProcessingPass.h src/Rendering/Pass/SkyboxPass.cpp src/Rendering/Pass/SkyboxPass.h src/Enums/ImageType.h src/Rendering/Pass/ZPrePass.cpp src/Rendering/Pass/ZPrePass.h src/Rendering/Pass/GBufferDebugPass.cpp src/Rendering/Pass/GBufferDebugPass.h src/Timer.cpp src/Timer.h src/GLObject/CreateInfo/SamplerCreateInfo.h src/GLObject/Sampler.cpp src/GLObject/Sampler.h src/Rendering/PostProcessingEffect/ExposureEffect.cpp src/Rendering/PostProcessingEffect/ExposureEffect.h src/Rendering/PostProcessingEffect/BloomEffect.cpp src/Rendering/PostProcessingEffect/BloomEffect.h src/Rendering/PostProcessingEffect/PostProcessingEffect.cpp src/Entity/Entity.cpp src/Entity/Entity.h src/Entity/Component/Component.h src/Iterator/ComponentIterator.h src/Iterator/ComponentConstIterator.h src/Entity/Component/Component.cpp src/Entity/Component/MeshRenderer.cpp src/Entity/Component/MeshRenderer.h src/Entity/Component/Animator.cpp src/Entity/Component/Animator.h src/Entity/ComponentSerialization.h src/Entity/Component/LightBase.cpp src/Entity/Component/LightBase.h src/Entity/Component/PointLight.cpp src/Entity/Component/PointLight.h src/Entity/Component/DirectionalLight.cpp src/Entity/Component/DirectionalLight.h src/Entity/EntitySerialization.h src/Iterator/ComponentIterator.cpp src/Iterator/ComponentConstIterator.cpp src/Iterator/EntityConstIterator.cpp src/Iterator/EntityConstIterator.h src/Iterator/EntityIterator.cpp src/Iterator/EntityIterator.h src/Event.h src/UI/Window/UIMenuBar.cpp src/UI/Window/UIMenuBar.h src/UI/Window/UIViewport.cpp src/UI/Window/UIViewport.h src/RenderContext.h)

set_property(TARGET Cyph3D PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_options(Cyph3D PRIVATE /Zc:__cplusplus)
target_compile_features(Cyph3D PRIVATE cxx_std_20)

find_path(STB_INCLUDE_DIRS "stb.h")
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_include_directories(Cyph3D PRIVATE
        "vendor/glad/include"
		"vendor/imgui/include"
		"vendor/thread-pool/include"
		"vendor/concurrentqueue/include"
		"vendor/imguizmo-f7bbbe3/include"
		${STB_INCLUDE_DIRS})

target_link_libraries(Cyph3D PRIVATE
		assimp::assimp
		glfw
		glm::glm
		magic_enum::magic_enum
		nlohmann_json nlohmann_json::nlohmann_json)

add_custom_command(TARGET Cyph3D PRE_BUILD
		COMMAND robocopy ${PROJECT_SOURCE_DIR}/resources/ ${PROJECT_BINARY_DIR}/resources /E /NS /NC /NFL /NDL /NP /NJH /NJS  || cmd /c "exit /b 0")