#pragma once

struct ImGuiContext;

class UIHelper
{
public:
	static void init();
	static void render();
	static void shutdown();
	static void onNewFrame();

private:
	static ImGuiContext* _context;
};
