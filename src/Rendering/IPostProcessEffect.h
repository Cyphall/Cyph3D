#pragma once

#include "../GLObject/Texture.h"

class IPostProcessEffect
{
public:
	virtual Texture* render(Texture* currentRenderResult, Texture& renderRaw, Texture& depth) = 0;
};