#include "BloomEffect.h"

#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/UI/Window/UIMisc.h"
#include "Cyph3D/Helper/RenderHelper.h"

#include <format>

static const float BLOOM_RADIUS = 0.85f;
static const float BLOOM_STRENGTH = 0.5f;

BloomEffect::BloomEffect(glm::ivec2 size):
PostProcessingEffect("Bloom", size),
_workTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F,
	.minFilter = GL_LINEAR_MIPMAP_NEAREST,
	.magFilter = GL_LINEAR,
	.wrapS = GL_CLAMP_TO_BORDER,
	.wrapT = GL_CLAMP_TO_BORDER,
	.borderColor = {0, 0, 0, 1},
	.levels = 0
}),
_outputTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F
}),
_downsampleProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/bloom/downsample.frag"}
}),
_upsampleAndBlurProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/bloom/upsample and blur.frag"}
}),
_composeProgram({
	{GL_VERTEX_SHADER, "internal/fullscreen quad.vert"},
	{GL_FRAGMENT_SHADER, "internal/post-processing/bloom/compose.frag"}
})
{
	_workFramebuffer.addToDrawBuffers(0, 0);

	_composeFramebuffer.attachColor(0, _outputTexture);
	_composeFramebuffer.addToDrawBuffers(0, 0);
}

GLTexture* BloomEffect::renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera)
{
	// copy input texture level 0 to work texture level 0
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "copyTextureBaseLevel");
	copyTextureBaseLevel(*currentRenderTexture, _workTexture);
	glPopDebugGroup();

	// downsample work texture
	for (int i = 0; i < _workTexture.getLevels() - 1; i++)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, std::format("downsample({})", i).c_str());
		downsample(_workTexture, i+1);
		glPopDebugGroup();
	}

	// upsample and blur work texture
	for (int i = 0; i < _workTexture.getLevels() - 1; i++)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, std::format("upsampleAndBlur({})", i).c_str());
		upsampleAndBlur(_workTexture, _workTexture.getLevels() - 2 - i, BLOOM_RADIUS);
		glPopDebugGroup();
	}

	// compose input texture level 0 and work texture level 0 to output texture level 0
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "compose");
	compose(*currentRenderTexture, _workTexture, BLOOM_STRENGTH);
	glPopDebugGroup();

	return &_outputTexture;
}

void BloomEffect::copyTextureBaseLevel(GLTexture& source, GLTexture& destination)
{
	glCopyImageSubData(
		source.getHandle(), GL_TEXTURE_2D, 0, 0, 0, 0,
		destination.getHandle(), GL_TEXTURE_2D, 0, 0, 0, 0,
		source.getSize().x, source.getSize().y, 1);
}

void BloomEffect::downsample(GLTexture& texture, int destLevel)
{
	glViewport(0, 0, _workTexture.getSize(destLevel).x, _workTexture.getSize(destLevel).y);

	_workFramebuffer.attachColor(0, texture, destLevel);
	_workFramebuffer.bindForDrawing();

	_downsampleProgram.setUniform("u_srcTexture", texture.getBindlessTextureHandle());
	_downsampleProgram.setUniform("u_srcLevel", destLevel-1);
	_downsampleProgram.setUniform("u_srcPixelSize", glm::vec2(1.0f) / glm::vec2(_workTexture.getSize(destLevel-1)));

	_downsampleProgram.bind();

	RenderHelper::drawScreenQuad();
}

void BloomEffect::upsampleAndBlur(GLTexture& texture, int destLevel, float bloomRadius)
{
	glViewport(0, 0, _workTexture.getSize(destLevel).x, _workTexture.getSize(destLevel).y);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	_workFramebuffer.attachColor(0, texture, destLevel);
	_workFramebuffer.bindForDrawing();

	_upsampleAndBlurProgram.setUniform("u_srcTexture", texture.getBindlessTextureHandle());
	_upsampleAndBlurProgram.setUniform("u_srcLevel", destLevel+1);
	_upsampleAndBlurProgram.setUniform("u_bloomRadius", bloomRadius);
	_upsampleAndBlurProgram.setUniform("u_srcPixelSize", glm::vec2(1.0f) / glm::vec2(_workTexture.getSize(destLevel+1)));

	_upsampleAndBlurProgram.bind();

	RenderHelper::drawScreenQuad();

	glDisable(GL_BLEND);
}

void BloomEffect::compose(GLTexture& sourceA, GLTexture& sourceB, float factor)
{
	glViewport(0, 0, sourceA.getSize().x, sourceA.getSize().y);

	_composeFramebuffer.bindForDrawing();

	_composeProgram.setUniform("u_srcATexture", sourceA.getBindlessTextureHandle());
	_composeProgram.setUniform("u_srcBTexture", sourceB.getBindlessTextureHandle());
	_composeProgram.setUniform("u_factor", factor);

	_composeProgram.bind();

	RenderHelper::drawScreenQuad();
}
