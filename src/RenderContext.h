#pragma once

class Renderer;
class Camera;

struct RenderContext
{
	Renderer& renderer;
	Camera& camera;
};