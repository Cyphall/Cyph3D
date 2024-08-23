cmake_minimum_required(VERSION 3.28)

cmake_path(CONVERT "$ENV{VCPKG_ROOT}" TO_CMAKE_PATH_LIST VCPKG_ROOT)

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set(CMAKE_ISPC_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/tools/ispc/ispc.exe")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

project(Cyph3D)

include(Shaders)
include(Resources)

add_compile_options($<$<COMPILE_LANGUAGE:C>:/Zc:__STDC__>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:/Zc:__cplusplus>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:/utf-8>)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Zc:__STDC__>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)

add_compile_options($<$<COMPILE_LANGUAGE:ISPC>:--arch=x86-64>)
add_compile_options($<$<COMPILE_LANGUAGE:ISPC>:--target=avx2-i32x16>)
add_compile_options($<$<COMPILE_LANGUAGE:ISPC>:--target-os=windows>)
add_compile_options($<$<COMPILE_LANGUAGE:ISPC>:--woff>)
add_compile_options($<$<COMPILE_LANGUAGE:ISPC>:--opt=fast-math>)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLFW_INCLUDE_NONE)
add_compile_definitions(VK_NO_PROTOTYPES)
add_compile_definitions(VULKAN_HPP_NO_SETTERS)
add_compile_definitions(VULKAN_HPP_NO_NODISCARD_WARNINGS)
add_compile_definitions(VULKAN_HPP_NO_SMART_HANDLE)
add_compile_definitions(VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

add_executable(Cyph3D)

target_sources(Cyph3D
		PRIVATE
			src/cpp/Cyph3D/Asset/AssetManager.cpp
			src/cpp/Cyph3D/Asset/BindlessTextureManager.cpp
			src/cpp/Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.cpp
			src/cpp/Cyph3D/Asset/Processing/AssetProcessor.cpp
			src/cpp/Cyph3D/Asset/Processing/EquirectangularSkyboxProcessor.cpp
			src/cpp/Cyph3D/Asset/Processing/ImageCompressor.cpp
			src/cpp/Cyph3D/Asset/Processing/ImageProcessor.cpp
			src/cpp/Cyph3D/Asset/Processing/MeshProcessor.cpp
			src/cpp/Cyph3D/Asset/RuntimeAsset/CubemapAsset.cpp
			src/cpp/Cyph3D/Asset/RuntimeAsset/MaterialAsset.cpp
			src/cpp/Cyph3D/Asset/RuntimeAsset/MeshAsset.cpp
			src/cpp/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.cpp
			src/cpp/Cyph3D/Asset/RuntimeAsset/TextureAsset.cpp
			src/cpp/Cyph3D/Engine.cpp
			src/cpp/Cyph3D/Entity/Component/Animator.cpp
			src/cpp/Cyph3D/Entity/Component/Component.cpp
			src/cpp/Cyph3D/Entity/Component/DirectionalLight.cpp
			src/cpp/Cyph3D/Entity/Component/LightBase.cpp
			src/cpp/Cyph3D/Entity/Component/ModelRenderer.cpp
			src/cpp/Cyph3D/Entity/Component/PointLight.cpp
			src/cpp/Cyph3D/Entity/Entity.cpp
			src/cpp/Cyph3D/Helper/FileHelper.cpp
			src/cpp/Cyph3D/Helper/ImGuiHelper.cpp
			src/cpp/Cyph3D/Helper/JsonHelper.cpp
			src/cpp/Cyph3D/Helper/MathHelper.cpp
			src/cpp/Cyph3D/Helper/StringHelper.cpp
			src/cpp/Cyph3D/Helper/ThreadHelper.cpp
			src/cpp/Cyph3D/Iterator/ComponentConstIterator.cpp
			src/cpp/Cyph3D/Iterator/ComponentIterator.cpp
			src/cpp/Cyph3D/Iterator/EntityConstIterator.cpp
			src/cpp/Cyph3D/Iterator/EntityIterator.cpp
			src/cpp/Cyph3D/LibImpl.cpp
			src/cpp/Cyph3D/Logging/Logger.cpp
			src/cpp/Cyph3D/Main.cpp
			src/cpp/Cyph3D/ObjectSerialization.cpp
			src/cpp/Cyph3D/Rendering/Pass/BloomPass.cpp
			src/cpp/Cyph3D/Rendering/Pass/ExposurePass.cpp
			src/cpp/Cyph3D/Rendering/Pass/LightingPass.cpp
			src/cpp/Cyph3D/Rendering/Pass/NormalizationPass.cpp
			src/cpp/Cyph3D/Rendering/Pass/PathTracePass.cpp
			src/cpp/Cyph3D/Rendering/Pass/ShadowMapPass.cpp
			src/cpp/Cyph3D/Rendering/Pass/SkyboxPass.cpp
			src/cpp/Cyph3D/Rendering/Pass/ToneMappingPass.cpp
			src/cpp/Cyph3D/Rendering/Pass/ZPrepass.cpp
			src/cpp/Cyph3D/Rendering/RenderRegistry.cpp
			src/cpp/Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.cpp
			src/cpp/Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.cpp
			src/cpp/Cyph3D/Rendering/SceneRenderer/SceneRenderer.cpp
			src/cpp/Cyph3D/Rendering/ShadowMapManager.cpp
			src/cpp/Cyph3D/Scene/Camera.cpp
			src/cpp/Cyph3D/Scene/Scene.cpp
			src/cpp/Cyph3D/Scene/Transform.cpp
			src/cpp/Cyph3D/StbImage.cpp
			src/cpp/Cyph3D/Timer.cpp
			src/cpp/Cyph3D/UI/ImGuiVulkanBackend.cpp
			src/cpp/Cyph3D/UI/ObjectPicker.cpp
			src/cpp/Cyph3D/UI/UIHelper.cpp
			src/cpp/Cyph3D/UI/Window/UIAssetBrowser.cpp
			src/cpp/Cyph3D/UI/Window/UIHierarchy.cpp
			src/cpp/Cyph3D/UI/Window/UIInspector.cpp
			src/cpp/Cyph3D/UI/Window/UIMenuBar.cpp
			src/cpp/Cyph3D/UI/Window/UIMisc.cpp
			src/cpp/Cyph3D/UI/Window/UIViewport.cpp
			src/cpp/Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.cpp
			src/cpp/Cyph3D/VKObject/Buffer/VKBufferBase.cpp
			src/cpp/Cyph3D/VKObject/Buffer/VKBufferInfo.cpp
			src/cpp/Cyph3D/VKObject/Buffer/VKResizableBufferInfo.cpp
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.cpp
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKRenderingColorAttachmentInfo.cpp
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKRenderingDepthAttachmentInfo.cpp
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.cpp
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.cpp
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.cpp
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.cpp
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.cpp
			src/cpp/Cyph3D/VKObject/Fence/VKFence.cpp
			src/cpp/Cyph3D/VKObject/Image/VKImage.cpp
			src/cpp/Cyph3D/VKObject/Image/VKImageInfo.cpp
			src/cpp/Cyph3D/VKObject/Image/VKSwapchainImage.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKComputePipeline.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKComputePipelineInfo.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipeline.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineAttachmentInfo.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineLayout.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineVertexInputLayoutInfo.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.cpp
			src/cpp/Cyph3D/VKObject/Pipeline/VKRayTracingPipelineInfo.cpp
			src/cpp/Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.cpp
			src/cpp/Cyph3D/VKObject/Queue/VKQueue.cpp
			src/cpp/Cyph3D/VKObject/Sampler/VKSampler.cpp
			src/cpp/Cyph3D/VKObject/Semaphore/VKSemaphore.cpp
			src/cpp/Cyph3D/VKObject/Shader/VKShader.cpp
			src/cpp/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTable.cpp
			src/cpp/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTableInfo.cpp
			src/cpp/Cyph3D/VKObject/VKContext.cpp
			src/cpp/Cyph3D/VKObject/VKHelper.cpp
			src/cpp/Cyph3D/VKObject/VKObject.cpp
			src/cpp/Cyph3D/VKObject/VKSwapchain.cpp
			src/cpp/Cyph3D/Window.cpp
		PRIVATE
		FILE_SET HEADERS
		BASE_DIRS src/cpp
		FILES
			src/cpp/Cyph3D/Asset/AssetManager.h
			src/cpp/Cyph3D/Asset/AssetManagerWorkerData.h
			src/cpp/Cyph3D/Asset/BindlessTextureManager.h
			src/cpp/Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.h
			src/cpp/Cyph3D/Asset/Processing/AssetProcessor.h
			src/cpp/Cyph3D/Asset/Processing/EquirectangularSkyboxData.h
			src/cpp/Cyph3D/Asset/Processing/EquirectangularSkyboxProcessor.h
			src/cpp/Cyph3D/Asset/Processing/ImageCompressor.h
			src/cpp/Cyph3D/Asset/Processing/ImageData.h
			src/cpp/Cyph3D/Asset/Processing/ImageProcessor.h
			src/cpp/Cyph3D/Asset/Processing/MeshData.h
			src/cpp/Cyph3D/Asset/Processing/MeshProcessor.h
			src/cpp/Cyph3D/Asset/RuntimeAsset/CubemapAsset.h
			src/cpp/Cyph3D/Asset/RuntimeAsset/GPUAsset.h
			src/cpp/Cyph3D/Asset/RuntimeAsset/MaterialAsset.h
			src/cpp/Cyph3D/Asset/RuntimeAsset/MeshAsset.h
			src/cpp/Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h
			src/cpp/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h
			src/cpp/Cyph3D/Asset/RuntimeAsset/TextureAsset.h
			src/cpp/Cyph3D/Engine.h
			src/cpp/Cyph3D/Entity/Component/Animator.h
			src/cpp/Cyph3D/Entity/Component/Component.h
			src/cpp/Cyph3D/Entity/Component/DirectionalLight.h
			src/cpp/Cyph3D/Entity/Component/LightBase.h
			src/cpp/Cyph3D/Entity/Component/ModelRenderer.h
			src/cpp/Cyph3D/Entity/Component/PointLight.h
			src/cpp/Cyph3D/Entity/Entity.h
			src/cpp/Cyph3D/Enums/ResourceType.h
			src/cpp/Cyph3D/GLSL_types.h
			src/cpp/Cyph3D/HashBuilder.h
			src/cpp/Cyph3D/Helper/FileHelper.h
			src/cpp/Cyph3D/Helper/ImGuiHelper.h
			src/cpp/Cyph3D/Helper/JsonHelper.h
			src/cpp/Cyph3D/Helper/MathHelper.h
			src/cpp/Cyph3D/Helper/StringHelper.h
			src/cpp/Cyph3D/Helper/ThreadHelper.h
			src/cpp/Cyph3D/Helper/VectorHelper.h
			src/cpp/Cyph3D/Iterator/ComponentConstIterator.h
			src/cpp/Cyph3D/Iterator/ComponentIterator.h
			src/cpp/Cyph3D/Iterator/EntityConstIterator.h
			src/cpp/Cyph3D/Iterator/EntityIterator.h
			src/cpp/Cyph3D/Logging/Logger.h
			src/cpp/Cyph3D/ObjectSerialization.h
			src/cpp/Cyph3D/Rendering/Pass/BloomPass.h
			src/cpp/Cyph3D/Rendering/Pass/ExposurePass.h
			src/cpp/Cyph3D/Rendering/Pass/LightingPass.h
			src/cpp/Cyph3D/Rendering/Pass/NormalizationPass.h
			src/cpp/Cyph3D/Rendering/Pass/PathTracePass.h
			src/cpp/Cyph3D/Rendering/Pass/RenderPass.h
			src/cpp/Cyph3D/Rendering/Pass/ShadowMapPass.h
			src/cpp/Cyph3D/Rendering/Pass/SkyboxPass.h
			src/cpp/Cyph3D/Rendering/Pass/ToneMappingPass.h
			src/cpp/Cyph3D/Rendering/Pass/ZPrepass.h
			src/cpp/Cyph3D/Rendering/RenderRegistry.h
			src/cpp/Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.h
			src/cpp/Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.h
			src/cpp/Cyph3D/Rendering/SceneRenderer/SceneRenderer.h
			src/cpp/Cyph3D/Rendering/ShadowMapManager.h
			src/cpp/Cyph3D/Rendering/VertexData.h
			src/cpp/Cyph3D/Scene/Camera.h
			src/cpp/Cyph3D/Scene/Scene.h
			src/cpp/Cyph3D/Scene/Transform.h
			src/cpp/Cyph3D/StbImage.h
			src/cpp/Cyph3D/Timer.h
			src/cpp/Cyph3D/UI/IInspectable.h
			src/cpp/Cyph3D/UI/ImGuiVulkanBackend.h
			src/cpp/Cyph3D/UI/ObjectPicker.h
			src/cpp/Cyph3D/UI/UIHelper.h
			src/cpp/Cyph3D/UI/Window/UIAssetBrowser.h
			src/cpp/Cyph3D/UI/Window/UIHierarchy.h
			src/cpp/Cyph3D/UI/Window/UIInspector.h
			src/cpp/Cyph3D/UI/Window/UIMenuBar.h
			src/cpp/Cyph3D/UI/Window/UIMisc.h
			src/cpp/Cyph3D/UI/Window/UIViewport.h
			src/cpp/Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h
			src/cpp/Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h
			src/cpp/Cyph3D/VKObject/AccelerationStructure/VKTopLevelAccelerationStructureBuildInfo.h
			src/cpp/Cyph3D/VKObject/Buffer/VKBuffer.h
			src/cpp/Cyph3D/VKObject/Buffer/VKBufferBase.h
			src/cpp/Cyph3D/VKObject/Buffer/VKBufferInfo.h
			src/cpp/Cyph3D/VKObject/Buffer/VKResizableBuffer.h
			src/cpp/Cyph3D/VKObject/Buffer/VKResizableBufferInfo.h
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKRenderingColorAttachmentInfo.h
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKRenderingDepthAttachmentInfo.h
			src/cpp/Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h
			src/cpp/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h
			src/cpp/Cyph3D/VKObject/Fence/VKFence.h
			src/cpp/Cyph3D/VKObject/Image/VKImage.h
			src/cpp/Cyph3D/VKObject/Image/VKImageInfo.h
			src/cpp/Cyph3D/VKObject/Image/VKSwapchainImage.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKComputePipeline.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKComputePipelineInfo.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipeline.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineAttachmentInfo.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineBlendingInfo.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineLayout.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineScissor.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineVertexInputLayoutInfo.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKPipelineViewport.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h
			src/cpp/Cyph3D/VKObject/Pipeline/VKRayTracingPipelineInfo.h
			src/cpp/Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.h
			src/cpp/Cyph3D/VKObject/Queue/VKQueue.h
			src/cpp/Cyph3D/VKObject/Sampler/VKSampler.h
			src/cpp/Cyph3D/VKObject/Semaphore/VKSemaphore.h
			src/cpp/Cyph3D/VKObject/Shader/VKShader.h
			src/cpp/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTable.h
			src/cpp/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTableInfo.h
			src/cpp/Cyph3D/VKObject/VKContext.h
			src/cpp/Cyph3D/VKObject/VKDynamic.h
			src/cpp/Cyph3D/VKObject/VKHelper.h
			src/cpp/Cyph3D/VKObject/VKObject.h
			src/cpp/Cyph3D/VKObject/VKSwapchain.h
			src/cpp/Cyph3D/Window.h
)

target_shaders(Cyph3D shaders "src/glsl"
		"src/glsl/fullscreen quad.vert"
		"src/glsl/asset processing/gen cubemap.comp"
		"src/glsl/asset processing/gen mipmap.comp"
		"src/glsl/imgui/imgui.frag"
		"src/glsl/imgui/imgui.vert"
		"src/glsl/lighting/lighting.frag"
		"src/glsl/lighting/lighting.vert"
		"src/glsl/object picker/object picker.frag"
		"src/glsl/object picker/object picker.vert"
		"src/glsl/path tracing/black skybox.rmiss"
		"src/glsl/path tracing/cubemap skybox.rmiss"
		"src/glsl/path tracing/path trace.rchit"
		"src/glsl/path tracing/path trace.rgen"
		"src/glsl/post-processing/bloom/compose.frag"
		"src/glsl/post-processing/bloom/downsample.frag"
		"src/glsl/post-processing/bloom/upsample and blur.frag"
		"src/glsl/post-processing/exposure/exposure.frag"
		"src/glsl/post-processing/normalization/normalization.comp"
		"src/glsl/post-processing/tone mapping/tone mapping.frag"
		"src/glsl/shadow mapping/directional light.vert"
		"src/glsl/shadow mapping/point light.frag"
		"src/glsl/shadow mapping/point light.vert"
		"src/glsl/skybox/skybox.frag"
		"src/glsl/skybox/skybox.vert"
		"src/glsl/z-prepass/z-prepass.vert"
)

target_resources(Cyph3D resources "resources"
		"resources/fonts/Font Awesome 6 Free-Solid-900.otf"
		"resources/fonts/Roboto-Regular.ttf"
)

target_precompile_headers(Cyph3D PRIVATE <vulkan/vulkan.hpp>)

find_package(Stb REQUIRED)
target_include_directories(Cyph3D PRIVATE ${Stb_INCLUDE_DIR})

find_package(assimp CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE assimp::assimp)

find_package(crossguid CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE crossguid)

find_package(Freetype REQUIRED)
target_link_libraries(Cyph3D PRIVATE Freetype::Freetype)

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

find_package(VulkanHeaders CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE Vulkan::Headers)

find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(Cyph3D PRIVATE GPUOpen::VulkanMemoryAllocator)

add_subdirectory("vendor/bs-thread-pool/v3.3.0")
target_link_libraries(Cyph3D PRIVATE bs-thread-pool)

add_subdirectory("vendor/imgui/v1.90-docking")
target_link_libraries(Cyph3D PRIVATE imgui)

add_subdirectory("vendor/imguizmo/fbc3614abcc642de7f3868cc0d68a35503ca9a4e")
target_link_libraries(Cyph3D PRIVATE imguizmo)

add_subdirectory("vendor/sigslot/v1.2.2")
target_link_libraries(Cyph3D PRIVATE sigslot)

add_subdirectory("vendor/ISPCTextureCompressor/691513b4fb406eccfc2f7d7f8213c8505ff5b897")
target_link_libraries(Cyph3D PRIVATE ISPCTextureCompressor)

add_subdirectory("vendor/half/v2.2.0")
target_link_libraries(Cyph3D PRIVATE half)

add_custom_target(copy_assets
		COMMAND robocopy ${PROJECT_SOURCE_DIR}/assets/ ${PROJECT_BINARY_DIR}/assets /XO /E /NS /NC /NFL /NDL /NP /NJH /NJS || cmd /c "exit /b 0")
add_dependencies(Cyph3D copy_assets)