#include "ZPrePass.h"
#include "../../Window.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"

ZPrePass::ZPrePass(std::unordered_map<std::string, Texture*>& textures) :
RenderPass(textures, "Z prepass"),
_framebuffer(Engine::getWindow().getSize()),
_depthTexture(TextureCreateInfo
{
	.size = _framebuffer.getSize(),
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

void ZPrePass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera)
{
	float clearDepth = 1;
	_depthTexture.clear(GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	
	_framebuffer.bindForDrawing();
	_vao.bind();
	_shader->bind();
	
	glm::mat4 vp = camera.getProjection() * camera.getView();
	
	for (int i = 0; i < registry.meshes.size(); i++)
	{
		MeshRenderer::RenderData data = registry.meshes[i];
		
		const Buffer<Mesh::VertexData>& vbo = data.mesh->getVBO();
		const Buffer<GLuint>& ibo = data.mesh->getIBO();
		
		_vao.bindBufferToSlot(vbo, 0);
		_vao.bindIndexBuffer(ibo);
		
		glm::mat4 mvp = vp * data.matrix;
		
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
