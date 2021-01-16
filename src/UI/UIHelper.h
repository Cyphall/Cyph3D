#pragma once

struct ImGuiContext;

class UIHelper
{
public:
	static void init();
	static void render();
	static void update(double deltaTime);
	static void shutdown();

private:
	static ImGuiContext* _context;
};
