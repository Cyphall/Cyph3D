cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ---- Project ----

project(
	Cyph3D
	VERSION 1.0.0
	LANGUAGES C CXX
)

# ---- Include guards ----

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif ()

# ---- Dependency: Cyphall/CyphGPU ----
find_package(CyphGPU CONFIG REQUIRED)
# ---- Dependency: ispc/ispc ----
find_program(ISPC_EXECUTABLE ispc REQUIRED)
# ---- Dependency: nothings/stb ----
find_package(Stb REQUIRED)
# ---- Dependency: assimp/assimp ----
find_package(assimp CONFIG REQUIRED)
# ---- Dependency: graeme-hill/crossguid ----
find_package(crossguid CONFIG REQUIRED)
# ---- Dependency: freetype/freetype ----
find_package(Freetype REQUIRED)
# ---- Dependency: glfw/glfw ----
find_package(glfw3 CONFIG REQUIRED)
# ---- Dependency: g-truc/glm ----
find_package(glm CONFIG REQUIRED)
# ---- Dependency: Neargye/magic_enum ----
find_package(magic_enum CONFIG REQUIRED)
# ---- Dependency: nlohmann/json ----
find_package(nlohmann_json CONFIG REQUIRED)
# ---- Dependency: SRombauts/SQLiteCpp ----
find_package(SQLiteCpp CONFIG REQUIRED)
# ---- Dependency: bshoshany/thread-pool ----
find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
# ---- Dependency: ocornut/imgui ----
find_package(imgui CONFIG REQUIRED)
# ---- Dependency: CedricGuillemet/ImGuizmo ----
find_package(imguizmo CONFIG REQUIRED)
# ---- Dependency: palacaze/sigslot ----
find_package(PalSigslot CONFIG REQUIRED)
# ---- Dependency: GameTechDev/ISPCTextureCompressor ----
find_package(ispc-texcomp CONFIG REQUIRED)
# ---- Dependency: half ----
find_path(HALF_INCLUDE_DIRS "half.hpp")
# ---- Dependency: gabime/spdlog ----
find_package(spdlog CONFIG REQUIRED)
# ---- Dependency: btzy/nativefiledialog-extended ----
find_package(nfd CONFIG REQUIRED)

# ---- Includes ----

include("cmake/utils/Resources.cmake")

# ---- Executable ----

add_executable(Cyph3D)

target_sources(Cyph3D PRIVATE
	"src/cpp/Cyph3D/Asset/AssetManager.cpp"
	"src/cpp/Cyph3D/Asset/AssetManagerWorkerData.cpp"
	"src/cpp/Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.cpp"
	"src/cpp/Cyph3D/Asset/Processing/AssetProcessor.cpp"
	"src/cpp/Cyph3D/Asset/Processing/EquirectangularSkyboxProcessor.cpp"
	"src/cpp/Cyph3D/Asset/Processing/ImageCompressor.cpp"
	"src/cpp/Cyph3D/Asset/Processing/ImageProcessor.cpp"
	"src/cpp/Cyph3D/Asset/Processing/MeshProcessor.cpp"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/CubemapAsset.cpp"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/MaterialAsset.cpp"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/MeshAsset.cpp"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.cpp"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/TextureAsset.cpp"
	"src/cpp/Cyph3D/Engine.cpp"
	"src/cpp/Cyph3D/Entity/Component/Animator.cpp"
	"src/cpp/Cyph3D/Entity/Component/Component.cpp"
	"src/cpp/Cyph3D/Entity/Component/DirectionalLight.cpp"
	"src/cpp/Cyph3D/Entity/Component/LightBase.cpp"
	"src/cpp/Cyph3D/Entity/Component/ModelRenderer.cpp"
	"src/cpp/Cyph3D/Entity/Component/PointLight.cpp"
	"src/cpp/Cyph3D/Entity/Entity.cpp"
	"src/cpp/Cyph3D/Helper/FileHelper.cpp"
	"src/cpp/Cyph3D/Helper/ImGuiHelper.cpp"
	"src/cpp/Cyph3D/Helper/JsonHelper.cpp"
	"src/cpp/Cyph3D/Helper/MathHelper.cpp"
	"src/cpp/Cyph3D/Iterator/ComponentConstIterator.cpp"
	"src/cpp/Cyph3D/Iterator/ComponentIterator.cpp"
	"src/cpp/Cyph3D/Iterator/EntityConstIterator.cpp"
	"src/cpp/Cyph3D/Iterator/EntityIterator.cpp"
	"src/cpp/Cyph3D/LibImpl.cpp"
	"src/cpp/Cyph3D/Main.cpp"
	"src/cpp/Cyph3D/ObjectSerialization.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/BloomPass.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/ExposurePass.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/LightingPass.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/NormalizationPass.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/PathTracePass.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/ShadowMapPass.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/ToneMappingPass.cpp"
	"src/cpp/Cyph3D/Rendering/Pass/ZPrepass.cpp"
	"src/cpp/Cyph3D/Rendering/RenderRegistry.cpp"
	"src/cpp/Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.cpp"
	"src/cpp/Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.cpp"
	"src/cpp/Cyph3D/Rendering/SceneRenderer/SceneRenderer.cpp"
	"src/cpp/Cyph3D/Rendering/ShadowMapManager.cpp"
	"src/cpp/Cyph3D/Scene/Camera.cpp"
	"src/cpp/Cyph3D/Scene/Scene.cpp"
	"src/cpp/Cyph3D/Scene/Transform.cpp"
	"src/cpp/Cyph3D/StbImage.cpp"
	"src/cpp/Cyph3D/Timer.cpp"
	"src/cpp/Cyph3D/UI/ObjectPicker.cpp"
	"src/cpp/Cyph3D/UI/UIHelper.cpp"
	"src/cpp/Cyph3D/UI/Window/UIAssetBrowser.cpp"
	"src/cpp/Cyph3D/UI/Window/UIHierarchy.cpp"
	"src/cpp/Cyph3D/UI/Window/UIInspector.cpp"
	"src/cpp/Cyph3D/UI/Window/UIMenuBar.cpp"
	"src/cpp/Cyph3D/UI/Window/UIMisc.cpp"
	"src/cpp/Cyph3D/UI/Window/UIViewport.cpp"
	"src/cpp/Cyph3D/Window.cpp"
)

target_sources(Cyph3D PRIVATE FILE_SET HEADERS BASE_DIRS "src/cpp" FILES
	"src/cpp/Cyph3D/Asset/AssetManager.h"
	"src/cpp/Cyph3D/Asset/AssetManagerWorkerData.h"
	"src/cpp/Cyph3D/Asset/Processing/AssetProcessingCacheDatabase.h"
	"src/cpp/Cyph3D/Asset/Processing/AssetProcessor.h"
	"src/cpp/Cyph3D/Asset/Processing/EquirectangularSkyboxData.h"
	"src/cpp/Cyph3D/Asset/Processing/EquirectangularSkyboxProcessor.h"
	"src/cpp/Cyph3D/Asset/Processing/ImageCompressor.h"
	"src/cpp/Cyph3D/Asset/Processing/ImageData.h"
	"src/cpp/Cyph3D/Asset/Processing/ImageProcessor.h"
	"src/cpp/Cyph3D/Asset/Processing/MeshData.h"
	"src/cpp/Cyph3D/Asset/Processing/MeshProcessor.h"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/RuntimeAsset.h"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
	"src/cpp/Cyph3D/Asset/RuntimeAsset/TextureAsset.h"
	"src/cpp/Cyph3D/Engine.h"
	"src/cpp/Cyph3D/Entity/Component/Animator.h"
	"src/cpp/Cyph3D/Entity/Component/Component.h"
	"src/cpp/Cyph3D/Entity/Component/DirectionalLight.h"
	"src/cpp/Cyph3D/Entity/Component/LightBase.h"
	"src/cpp/Cyph3D/Entity/Component/ModelRenderer.h"
	"src/cpp/Cyph3D/Entity/Component/PointLight.h"
	"src/cpp/Cyph3D/Entity/Entity.h"
	"src/cpp/Cyph3D/Enums/ResourceType.h"
	"src/cpp/Cyph3D/HashBuilder.h"
	"src/cpp/Cyph3D/Helper/FileHelper.h"
	"src/cpp/Cyph3D/Helper/ImGuiHelper.h"
	"src/cpp/Cyph3D/Helper/JsonHelper.h"
	"src/cpp/Cyph3D/Helper/MathHelper.h"
	"src/cpp/Cyph3D/Helper/VectorHelper.h"
	"src/cpp/Cyph3D/Iterator/ComponentConstIterator.h"
	"src/cpp/Cyph3D/Iterator/ComponentIterator.h"
	"src/cpp/Cyph3D/Iterator/EntityConstIterator.h"
	"src/cpp/Cyph3D/Iterator/EntityIterator.h"
	"src/cpp/Cyph3D/ObjectSerialization.h"
	"src/cpp/Cyph3D/Rendering/Pass/BloomPass.h"
	"src/cpp/Cyph3D/Rendering/Pass/ExposurePass.h"
	"src/cpp/Cyph3D/Rendering/Pass/LightingPass.h"
	"src/cpp/Cyph3D/Rendering/Pass/NormalizationPass.h"
	"src/cpp/Cyph3D/Rendering/Pass/PathTracePass.h"
	"src/cpp/Cyph3D/Rendering/Pass/RenderPass.h"
	"src/cpp/Cyph3D/Rendering/Pass/ShadowMapPass.h"
	"src/cpp/Cyph3D/Rendering/Pass/ToneMappingPass.h"
	"src/cpp/Cyph3D/Rendering/Pass/ZPrepass.h"
	"src/cpp/Cyph3D/Rendering/RenderRegistry.h"
	"src/cpp/Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.h"
	"src/cpp/Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.h"
	"src/cpp/Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
	"src/cpp/Cyph3D/Rendering/ShadowMapManager.h"
	"src/cpp/Cyph3D/Rendering/VertexData.h"
	"src/cpp/Cyph3D/Scene/Camera.h"
	"src/cpp/Cyph3D/Scene/Scene.h"
	"src/cpp/Cyph3D/Scene/Transform.h"
	"src/cpp/Cyph3D/StbImage.h"
	"src/cpp/Cyph3D/Timer.h"
	"src/cpp/Cyph3D/UI/IInspectable.h"
	"src/cpp/Cyph3D/UI/ObjectPicker.h"
	"src/cpp/Cyph3D/UI/UIHelper.h"
	"src/cpp/Cyph3D/UI/Window/UIAssetBrowser.h"
	"src/cpp/Cyph3D/UI/Window/UIHierarchy.h"
	"src/cpp/Cyph3D/UI/Window/UIInspector.h"
	"src/cpp/Cyph3D/UI/Window/UIMenuBar.h"
	"src/cpp/Cyph3D/UI/Window/UIMisc.h"
	"src/cpp/Cyph3D/UI/Window/UIViewport.h"
	"src/cpp/Cyph3D/Window.h"
)

cgpu_target_shader_pipelines(Cyph3D shaders "src/slang"
	"src/slang/Cyph3D/asset processing/gen cubemap.slang"
	"src/slang/Cyph3D/lighting/object.slang"
	"src/slang/Cyph3D/lighting/skybox.slang"
	"src/slang/Cyph3D/object picker/object picker.slang"
	"src/slang/Cyph3D/path tracing/trace.slang"
	"src/slang/Cyph3D/post-processing/bloom/downsample.slang"
	"src/slang/Cyph3D/post-processing/bloom/upsample.slang"
	"src/slang/Cyph3D/post-processing/exposure/exposure.slang"
	"src/slang/Cyph3D/post-processing/normalization/normalization.slang"
	"src/slang/Cyph3D/post-processing/tone mapping/tone mapping.slang"
	"src/slang/Cyph3D/shadow mapping/directional light.slang"
	"src/slang/Cyph3D/shadow mapping/point light.slang"
	"src/slang/Cyph3D/z-prepass/z-prepass.slang"
)

target_resources(Cyph3D resources "resources"
	"resources/fonts/Font Awesome 6 Free-Solid-900.otf"
	"resources/fonts/Roboto-Regular.ttf"
)

add_custom_command(TARGET Cyph3D POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${PROJECT_SOURCE_DIR}/assets" "${PROJECT_BINARY_DIR}/assets"
)

target_precompile_headers(Cyph3D PRIVATE <vulkan/vulkan.hpp>)

set_target_properties(Cyph3D PROPERTIES CXX_STANDARD 23)
set_target_properties(Cyph3D PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(Cyph3D PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(Cyph3D PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
set_target_properties(Cyph3D PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_target_properties(Cyph3D PROPERTIES DEBUG_POSTFIX d)
set_target_properties(Cyph3D PROPERTIES COMPILE_WARNING_AS_ERROR $<CONFIG:Release>)
#set_target_properties(Cyph3D PROPERTIES WIN32_EXECUTABLE ON)

# ---- Install ----

install(TARGETS Cyph3D)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/assets"
	TYPE BIN
)

# ---- Dependency: Cyphall/CyphGPU ----
target_link_libraries(Cyph3D PRIVATE CyphGPU)
# ---- Dependency: ispc/ispc ----
set(CMAKE_ISPC_COMPILER "${ISPC_EXECUTABLE}")
# ---- Dependency: nothings/stb ----
target_include_directories(Cyph3D PRIVATE ${Stb_INCLUDE_DIR})
# ---- Dependency: assimp/assimp ----
target_link_libraries(Cyph3D PRIVATE assimp::assimp)
# ---- Dependency: graeme-hill/crossguid ----
target_link_libraries(Cyph3D PRIVATE crossguid)
# ---- Dependency: freetype/freetype ----
target_link_libraries(Cyph3D PRIVATE Freetype::Freetype)
# ---- Dependency: glfw/glfw ----
target_link_libraries(Cyph3D PRIVATE glfw)
target_compile_definitions(Cyph3D PRIVATE GLFW_INCLUDE_NONE)
# ---- Dependency: g-truc/glm ----
target_link_libraries(Cyph3D PRIVATE glm::glm)
target_compile_definitions(Cyph3D PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(Cyph3D PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)
# ---- Dependency: Neargye/magic_enum ----
target_link_libraries(Cyph3D PRIVATE magic_enum::magic_enum)
# ---- Dependency: nlohmann/json ----
target_link_libraries(Cyph3D PRIVATE nlohmann_json::nlohmann_json)
# ---- Dependency: SRombauts/SQLiteCpp ----
target_link_libraries(Cyph3D PRIVATE SQLiteCpp)
# ---- Dependency: bshoshany/thread-pool ----
target_include_directories(Cyph3D PRIVATE ${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS})
target_compile_definitions(Cyph3D PRIVATE BS_THREAD_POOL_NATIVE_EXTENSIONS)
# ---- Dependency: ocornut/imgui ----
target_link_libraries(Cyph3D PRIVATE imgui)
# ---- Dependency: CedricGuillemet/ImGuizmo ----
target_link_libraries(Cyph3D PRIVATE imguizmo)
# ---- Dependency: palacaze/sigslot ----
target_link_libraries(Cyph3D PRIVATE Pal::Sigslot)
# ---- Dependency: GameTechDev/ISPCTextureCompressor ----
target_link_libraries(Cyph3D PRIVATE ispc-texcomp)
# ---- Dependency: half ----
target_include_directories(Cyph3D PRIVATE ${HALF_INCLUDE_DIRS})
# ---- Dependency: gabime/spdlog ----
target_link_libraries(Cyph3D PRIVATE spdlog::spdlog)
# ---- Dependency: btzy/nativefiledialog-extended ----
target_link_libraries(Cyph3D PRIVATE nfd::nfd)
