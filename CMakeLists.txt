cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set(CMAKE_ISPC_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/tools/ispc/ispc.exe")
set(CMAKE_CXX_STANDARD 20)

cmake_policy(SET CMP0091 NEW)

project(Cyph3D)

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

add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLFW_INCLUDE_NONE)
add_compile_definitions(VK_NO_PROTOTYPES)
add_compile_definitions(VULKAN_HPP_NO_SETTERS)
add_compile_definitions(VULKAN_HPP_NO_NODISCARD_WARNINGS)
add_compile_definitions(VULKAN_HPP_NO_SMART_HANDLE)
add_compile_definitions(VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

add_executable(Cyph3D
		src/Cyph3D/Engine.cpp
		src/Cyph3D/Engine.h
		src/Cyph3D/GLSL_types.h
		src/Cyph3D/HashBuilder.h
		src/Cyph3D/LibImpl.cpp
		src/Cyph3D/Main.cpp
		src/Cyph3D/ObjectSerialization.cpp
		src/Cyph3D/ObjectSerialization.h
		src/Cyph3D/StbImage.cpp
		src/Cyph3D/StbImage.h
		src/Cyph3D/Timer.cpp
		src/Cyph3D/Timer.h
		src/Cyph3D/Window.cpp
		src/Cyph3D/Window.h
		src/Cyph3D/Asset/AssetManager.cpp
		src/Cyph3D/Asset/AssetManager.h
		src/Cyph3D/Asset/AssetManagerWorkerData.h
		src/Cyph3D/Asset/BindlessTextureManager.cpp
		src/Cyph3D/Asset/BindlessTextureManager.h
		src/Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.cpp
		src/Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.h
		src/Cyph3D/Asset/Processing/AssetProcessor.cpp
		src/Cyph3D/Asset/Processing/AssetProcessor.h
		src/Cyph3D/Asset/Processing/ImageData.h
		src/Cyph3D/Asset/Processing/ImageProcessor.cpp
		src/Cyph3D/Asset/Processing/ImageProcessor.h
		src/Cyph3D/Asset/Processing/MeshData.h
		src/Cyph3D/Asset/Processing/MeshProcessor.cpp
		src/Cyph3D/Asset/Processing/MeshProcessor.h
		src/Cyph3D/Asset/RuntimeAsset/CubemapAsset.cpp
		src/Cyph3D/Asset/RuntimeAsset/CubemapAsset.h
		src/Cyph3D/Asset/RuntimeAsset/GPUAsset.h
		src/Cyph3D/Asset/RuntimeAsset/MaterialAsset.cpp
		src/Cyph3D/Asset/RuntimeAsset/MaterialAsset.h
		src/Cyph3D/Asset/RuntimeAsset/MeshAsset.cpp
		src/Cyph3D/Asset/RuntimeAsset/MeshAsset.h
		src/Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h
		src/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.cpp
		src/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h
		src/Cyph3D/Asset/RuntimeAsset/TextureAsset.cpp
		src/Cyph3D/Asset/RuntimeAsset/TextureAsset.h
		src/Cyph3D/Entity/Entity.cpp
		src/Cyph3D/Entity/Entity.h
		src/Cyph3D/Entity/Component/Animator.cpp
		src/Cyph3D/Entity/Component/Animator.h
		src/Cyph3D/Entity/Component/Component.cpp
		src/Cyph3D/Entity/Component/Component.h
		src/Cyph3D/Entity/Component/DirectionalLight.cpp
		src/Cyph3D/Entity/Component/DirectionalLight.h
		src/Cyph3D/Entity/Component/LightBase.cpp
		src/Cyph3D/Entity/Component/LightBase.h
		src/Cyph3D/Entity/Component/ModelRenderer.cpp
		src/Cyph3D/Entity/Component/ModelRenderer.h
		src/Cyph3D/Entity/Component/PointLight.cpp
		src/Cyph3D/Entity/Component/PointLight.h
		src/Cyph3D/Enums/ResourceType.h
		src/Cyph3D/Helper/FileHelper.cpp
		src/Cyph3D/Helper/FileHelper.h
		src/Cyph3D/Helper/ImGuiHelper.cpp
		src/Cyph3D/Helper/ImGuiHelper.h
		src/Cyph3D/Helper/JsonHelper.cpp
		src/Cyph3D/Helper/JsonHelper.h
		src/Cyph3D/Helper/MathHelper.cpp
		src/Cyph3D/Helper/MathHelper.h
		src/Cyph3D/Helper/StringHelper.cpp
		src/Cyph3D/Helper/StringHelper.h
		src/Cyph3D/Helper/ThreadHelper.cpp
		src/Cyph3D/Helper/ThreadHelper.h
		src/Cyph3D/Helper/VectorHelper.h
		src/Cyph3D/Iterator/ComponentConstIterator.cpp
		src/Cyph3D/Iterator/ComponentConstIterator.h
		src/Cyph3D/Iterator/ComponentIterator.cpp
		src/Cyph3D/Iterator/ComponentIterator.h
		src/Cyph3D/Iterator/EntityConstIterator.cpp
		src/Cyph3D/Iterator/EntityConstIterator.h
		src/Cyph3D/Iterator/EntityIterator.cpp
		src/Cyph3D/Iterator/EntityIterator.h
		src/Cyph3D/Logging/Logger.cpp
		src/Cyph3D/Logging/Logger.h
		src/Cyph3D/Rendering/RenderRegistry.cpp
		src/Cyph3D/Rendering/RenderRegistry.h
		src/Cyph3D/Rendering/ShadowMapManager.cpp
		src/Cyph3D/Rendering/ShadowMapManager.h
		src/Cyph3D/Rendering/VertexData.h
		src/Cyph3D/Rendering/Pass/BloomPass.cpp
		src/Cyph3D/Rendering/Pass/BloomPass.h
		src/Cyph3D/Rendering/Pass/ExposurePass.cpp
		src/Cyph3D/Rendering/Pass/ExposurePass.h
		src/Cyph3D/Rendering/Pass/LightingPass.cpp
		src/Cyph3D/Rendering/Pass/LightingPass.h
		src/Cyph3D/Rendering/Pass/NormalizationPass.cpp
		src/Cyph3D/Rendering/Pass/NormalizationPass.h
		src/Cyph3D/Rendering/Pass/PathTracePass.cpp
		src/Cyph3D/Rendering/Pass/PathTracePass.h
		src/Cyph3D/Rendering/Pass/RenderPass.h
		src/Cyph3D/Rendering/Pass/ShadowMapPass.cpp
		src/Cyph3D/Rendering/Pass/ShadowMapPass.h
		src/Cyph3D/Rendering/Pass/SkyboxPass.cpp
		src/Cyph3D/Rendering/Pass/SkyboxPass.h
		src/Cyph3D/Rendering/Pass/ToneMappingPass.cpp
		src/Cyph3D/Rendering/Pass/ToneMappingPass.h
		src/Cyph3D/Rendering/Pass/ZPrepass.cpp
		src/Cyph3D/Rendering/Pass/ZPrepass.h
		src/Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.cpp
		src/Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.h
		src/Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.cpp
		src/Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.h
		src/Cyph3D/Rendering/SceneRenderer/SceneRenderer.cpp
		src/Cyph3D/Rendering/SceneRenderer/SceneRenderer.h
		src/Cyph3D/Scene/Camera.cpp
		src/Cyph3D/Scene/Camera.h
		src/Cyph3D/Scene/Scene.cpp
		src/Cyph3D/Scene/Scene.h
		src/Cyph3D/Scene/Transform.cpp
		src/Cyph3D/Scene/Transform.h
		src/Cyph3D/UI/IInspectable.h
		src/Cyph3D/UI/ImGuiVulkanBackend.cpp
		src/Cyph3D/UI/ImGuiVulkanBackend.h
		src/Cyph3D/UI/ObjectPicker.cpp
		src/Cyph3D/UI/ObjectPicker.h
		src/Cyph3D/UI/UIHelper.cpp
		src/Cyph3D/UI/UIHelper.h
		src/Cyph3D/UI/Window/UIAssetBrowser.cpp
		src/Cyph3D/UI/Window/UIAssetBrowser.h
		src/Cyph3D/UI/Window/UIHierarchy.cpp
		src/Cyph3D/UI/Window/UIHierarchy.h
		src/Cyph3D/UI/Window/UIInspector.cpp
		src/Cyph3D/UI/Window/UIInspector.h
		src/Cyph3D/UI/Window/UIMenuBar.cpp
		src/Cyph3D/UI/Window/UIMenuBar.h
		src/Cyph3D/UI/Window/UIMisc.cpp
		src/Cyph3D/UI/Window/UIMisc.h
		src/Cyph3D/UI/Window/UIViewport.cpp
		src/Cyph3D/UI/Window/UIViewport.h
		src/Cyph3D/VKObject/VKContext.cpp
		src/Cyph3D/VKObject/VKContext.h
		src/Cyph3D/VKObject/VKDynamic.h
		src/Cyph3D/VKObject/VKHelper.cpp
		src/Cyph3D/VKObject/VKHelper.h
		src/Cyph3D/VKObject/VKObject.cpp
		src/Cyph3D/VKObject/VKObject.h
		src/Cyph3D/VKObject/VKPtr.h
		src/Cyph3D/VKObject/VKSwapchain.cpp
		src/Cyph3D/VKObject/VKSwapchain.h
		src/Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.cpp
		src/Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h
		src/Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h
		src/Cyph3D/VKObject/AccelerationStructure/VKTopLevelAccelerationStructureBuildInfo.h
		src/Cyph3D/VKObject/Buffer/VKBuffer.h
		src/Cyph3D/VKObject/Buffer/VKBufferBase.h
		src/Cyph3D/VKObject/Buffer/VKBufferInfo.cpp
		src/Cyph3D/VKObject/Buffer/VKBufferInfo.h
		src/Cyph3D/VKObject/Buffer/VKResizableBuffer.h
		src/Cyph3D/VKObject/Buffer/VKResizableBufferInfo.cpp
		src/Cyph3D/VKObject/Buffer/VKResizableBufferInfo.h
		src/Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.cpp
		src/Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h
		src/Cyph3D/VKObject/CommandBuffer/VKRenderingColorAttachmentInfo.cpp
		src/Cyph3D/VKObject/CommandBuffer/VKRenderingColorAttachmentInfo.h
		src/Cyph3D/VKObject/CommandBuffer/VKRenderingDepthAttachmentInfo.cpp
		src/Cyph3D/VKObject/CommandBuffer/VKRenderingDepthAttachmentInfo.h
		src/Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.cpp
		src/Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.cpp
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.cpp
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetInfo.h
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.cpp
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.cpp
		src/Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h
		src/Cyph3D/VKObject/Fence/VKFence.cpp
		src/Cyph3D/VKObject/Fence/VKFence.h
		src/Cyph3D/VKObject/Image/VKImage.cpp
		src/Cyph3D/VKObject/Image/VKImage.h
		src/Cyph3D/VKObject/Image/VKImageInfo.cpp
		src/Cyph3D/VKObject/Image/VKImageInfo.h
        src/Cyph3D/VKObject/Image/VKSwapchainImage.cpp
		src/Cyph3D/VKObject/Image/VKSwapchainImage.h
		src/Cyph3D/VKObject/Pipeline/VKComputePipeline.cpp
		src/Cyph3D/VKObject/Pipeline/VKComputePipeline.h
		src/Cyph3D/VKObject/Pipeline/VKComputePipelineInfo.cpp
		src/Cyph3D/VKObject/Pipeline/VKComputePipelineInfo.h
		src/Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.cpp
		src/Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h
		src/Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.cpp
		src/Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h
		src/Cyph3D/VKObject/Pipeline/VKPipeline.cpp
		src/Cyph3D/VKObject/Pipeline/VKPipeline.h
		src/Cyph3D/VKObject/Pipeline/VKPipelineAttachmentInfo.cpp
		src/Cyph3D/VKObject/Pipeline/VKPipelineAttachmentInfo.h
		src/Cyph3D/VKObject/Pipeline/VKPipelineBlendingInfo.h
		src/Cyph3D/VKObject/Pipeline/VKPipelineLayout.cpp
		src/Cyph3D/VKObject/Pipeline/VKPipelineLayout.h
		src/Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.cpp
		src/Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h
		src/Cyph3D/VKObject/Pipeline/VKPipelineScissor.h
		src/Cyph3D/VKObject/Pipeline/VKPipelineVertexInputLayoutInfo.cpp
		src/Cyph3D/VKObject/Pipeline/VKPipelineVertexInputLayoutInfo.h
		src/Cyph3D/VKObject/Pipeline/VKPipelineViewport.h
		src/Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.cpp
		src/Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h
		src/Cyph3D/VKObject/Pipeline/VKRayTracingPipelineInfo.cpp
		src/Cyph3D/VKObject/Pipeline/VKRayTracingPipelineInfo.h
		src/Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.cpp
		src/Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.h
		src/Cyph3D/VKObject/Queue/VKQueue.cpp
		src/Cyph3D/VKObject/Queue/VKQueue.h
		src/Cyph3D/VKObject/Sampler/VKSampler.cpp
		src/Cyph3D/VKObject/Sampler/VKSampler.h
		src/Cyph3D/VKObject/Semaphore/VKSemaphore.cpp
		src/Cyph3D/VKObject/Semaphore/VKSemaphore.h
		src/Cyph3D/VKObject/Shader/VKShader.cpp
		src/Cyph3D/VKObject/Shader/VKShader.h
		src/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTable.cpp
		src/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTable.h
		src/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTableInfo.cpp
		src/Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTableInfo.h
		src/Cyph3D/Asset/Processing/ImageCompressor.cpp
		src/Cyph3D/Asset/Processing/ImageCompressor.h
		src/Cyph3D/Asset/Processing/EquirectangularSkyboxProcessor.cpp
		src/Cyph3D/Asset/Processing/EquirectangularSkyboxProcessor.h
		src/Cyph3D/Asset/Processing/EquirectangularSkyboxData.h
		src/Cyph3D/VKObject/Buffer/VKBufferBase.cpp
)

target_precompile_headers(Cyph3D PRIVATE <vulkan/vulkan.hpp>)

target_include_directories(Cyph3D PRIVATE "src")

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

find_package(Vulkan REQUIRED)
target_link_libraries(Cyph3D PRIVATE Vulkan::Headers)

add_subdirectory("vendor/bs-thread-pool/v3.3.0")
target_link_libraries(Cyph3D PRIVATE bs-thread-pool)

add_subdirectory("vendor/imgui/v1.90-docking")
target_link_libraries(Cyph3D PRIVATE imgui)

add_subdirectory("vendor/imguizmo/fbc3614abcc642de7f3868cc0d68a35503ca9a4e")
target_link_libraries(Cyph3D PRIVATE imguizmo)

add_subdirectory("vendor/VulkanMemoryAllocator/5677097bafb8477097c6e3354ce68b7a44fd01a4")
target_link_libraries(Cyph3D PRIVATE VulkanMemoryAllocator)

add_subdirectory("vendor/sigslot/v1.2.2")
target_link_libraries(Cyph3D PRIVATE sigslot)

add_subdirectory("vendor/ISPCTextureCompressor/691513b4fb406eccfc2f7d7f8213c8505ff5b897")
target_link_libraries(Cyph3D PRIVATE ISPCTextureCompressor)

add_subdirectory("vendor/half/v2.2.0")
target_link_libraries(Cyph3D PRIVATE half)

find_package(Python COMPONENTS Interpreter)
add_custom_target(copy_data
		COMMAND ${Python_EXECUTABLE} "${PROJECT_SOURCE_DIR}/copy_resources.py" "${PROJECT_SOURCE_DIR}/data" "${PROJECT_BINARY_DIR}/data" "${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}" "${CMAKE_BUILD_TYPE}")
add_dependencies(Cyph3D copy_data)

add_custom_target(copy_resources
		COMMAND robocopy ${PROJECT_SOURCE_DIR}/resources/ ${PROJECT_BINARY_DIR}/resources /XO /E /NS /NC /NFL /NDL /NP /NJH /NJS || cmd /c "exit /b 0")
add_dependencies(Cyph3D copy_resources)