#include "ZPrePass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/Mesh.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/Shape/Shape.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Window.h"

ZPrePass::ZPrePass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size) :
RenderPass(textures, size, "Z prepass"),
_framebuffer(size),
_depthTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_DEPTH_COMPONENT24
})
{
	_framebuffer.attachDepth(_depthTexture);
	
	textures["z-prepass_depth"] = &_depthTexture;
	
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/z-prepass/z-prepass");
	
	_shader = Engine::getGlobalRM().requestShaderProgram(createInfo);
	
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
}

void ZPrePass::preparePipelineImpl()
{
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_CULL_FACE);
}

void ZPrePass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep)
{
	float clearDepth = 1;
	_depthTexture.clear(GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	
	_framebuffer.bindForDrawing();
	_vao.bind();
	_shader->bind();
	
	glm::mat4 vp = camera.getProjection() * camera.getView();
	
	for (int i = 0; i < registry.shapes.size(); i++)
	{
		ShapeRenderer::RenderData shapeData = registry.shapes[i];
		
		if (!shapeData.shape->isReadyForRasterisationRender())
			continue;
		
		const Mesh& mesh = shapeData.shape->getMeshToRender();
		
		const Buffer<Mesh::VertexData>& vbo = mesh.getVBO();
		const Buffer<GLuint>& ibo = mesh.getIBO();
		
		_vao.bindBufferToSlot(vbo, 0);
		_vao.bindIndexBuffer(ibo);
		
		glm::mat4 mvp = vp * shapeData.matrix;
		
		_shader->setUniform("u_mvp", mvp);
		
		glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
	}
}

void ZPrePass::restorePipelineImpl()
{
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_CULL_FACE);
}