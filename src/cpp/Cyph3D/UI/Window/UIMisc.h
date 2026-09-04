#pragma once

#include <Cyph3D/Rendering/RenderRegistry.h>
#include <Cyph3D/Scene/Camera.h>

#include <atomic>
#include <CyphGPU/fwd.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <thread>

namespace c3d
{
class UIMisc
{
public:
	static void show();

	static bool isSimulationEnabled();
	static int viewportSampleCount();

	static bool isRenderToFileInProgress();

private:
	struct RenderToFileState
	{
		std::atomic_uint renderedSamples{0};
		std::atomic_uint totalSamples{};
		Camera camera{};
		RenderRegistry registry{};
		std::filesystem::path outputFile{};
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime{};
		std::atomic<std::chrono::time_point<std::chrono::high_resolution_clock>> lastTraceTime{};

		std::atomic_bool forceFinish{false};

		std::atomic_bool finished{false};
	};

	struct RenderToFileData
	{
		RenderToFileState state{};
		std::jthread thread{};
	};

	static glm::ivec2 _resolution;
	static uint32_t _renderSampleCount;
	static bool _simulationEnabled;
	static int _viewportSampleCount;

	static std::array<float, 512> _frametimes;
	static uint32_t _lastFrametimeIndex;
	static float _overlayFrametime;
	static float _timeUntilOverlayUpdate;

	static std::optional<RenderToFileData> _renderToFileData;

	static void displayFrametime();

	static void renderToFile(glm::uvec2 resolution, uint32_t sampleCount);
};
}
