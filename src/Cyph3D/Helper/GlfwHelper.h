#pragma once

class GlfwHelper
{
public:
	static void Init();

private:
	static void EnsureGpuIsCompatible();
};