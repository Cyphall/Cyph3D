#pragma once

class SceneRenderer;
class Camera;

struct RenderContext
{
	SceneRenderer& renderer;
	Camera& camera;
};