#include "ShadowMapPass.h"
#include "../../Window.h"
#include "../../Engine.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../GLObject/Mesh.h"

ShadowMapPass::ShadowMapPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size):
RenderPass(textures, size, "Shadow map pass")
{
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
	
	ShaderProgramCreateInfo directionalLightCreateInfo;
	directionalLightCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadow mapping/directional light");
	
	_directionalLightShadowMappingProgram = Engine::getGlobalRM().requestShaderProgram(directionalLightCreateInfo);
	
	ShaderProgramCreateInfo pointLightCreateInfo;
	pointLightCreateInfo.shadersFiles[GL_GEOMETRY_SHADER].emplace_back("internal/shadow mapping/point light");
	pointLightCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadow mapping/point light");
	pointLightCreateInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/shadow mapping/point light");
	
	_pointLightShadowMappingProgram = Engine::getGlobalRM().requestShaderProgram(pointLightCreateInfo);
}

void ShadowMapPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void ShadowMapPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera)
{
	_vao.bind();
	
	_directionalLightShadowMappingProgram->bind();
	for (DirectionalLight::RenderData& renderData : registry.directionalLights)
	{
		if (!renderData.castShadows) continue;
		
		glViewport(0, 0, renderData.mapResolution, renderData.mapResolution);
		
		renderData.shadowMapFramebuffer->bindForDrawing();
		
		float depthColor = 1;
		renderData.shadowMapTexture->clear(GL_DEPTH_COMPONENT, GL_FLOAT, &depthColor);
		
		for (auto& shapeData : registry.shapes)
		{
			if (!shapeData.contributeShadows) continue;
			
			if (!shapeData.shape->isReadyForRasterisationRender())
				continue;
			
			const Mesh& mesh = shapeData.shape->getMeshToRender();
			
			const Buffer<Mesh::VertexData>& vbo = mesh.getVBO();
			const Buffer<GLuint>& ibo = mesh.getIBO();
			_vao.bindBufferToSlot(vbo, 0);
			_vao.bindIndexBuffer(ibo);
			
			glm::mat4 mvp = renderData.lightViewProjection * shapeData.matrix;
			
			_directionalLightShadowMappingProgram->setUniform("u_mvp", mvp);
			
			glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
		}
	}
	
	_pointLightShadowMappingProgram->bind();
	for (PointLight::RenderData& renderData : registry.pointLights)
	{
		if (!renderData.castShadows) return;
		
		glViewport(0, 0, renderData.mapResolution, renderData.mapResolution);
		
		glm::vec3 worldPos = renderData.pos;
		
		renderData.shadowMapFramebuffer->bindForDrawing();
		_pointLightShadowMappingProgram->bind();
		_pointLightShadowMappingProgram->setUniform("u_viewProjections", renderData.viewProjections, 6);
		_pointLightShadowMappingProgram->setUniform("u_lightPos", worldPos);
		_pointLightShadowMappingProgram->setUniform("u_far", renderData.far);
		
		float depthColor = 1;
		renderData.shadowMapTexture->clear(GL_DEPTH_COMPONENT, GL_FLOAT, &depthColor);
		
		for (auto& shapeData : registry.shapes)
		{
			if (!shapeData.contributeShadows) continue;
			
			if (!shapeData.shape->isReadyForRasterisationRender())
				continue;
			
			const Mesh& mesh = shapeData.shape->getMeshToRender();
			
			const Buffer<Mesh::VertexData>& vbo = mesh.getVBO();
			const Buffer<GLuint>& ibo = mesh.getIBO();
			_vao.bindBufferToSlot(vbo, 0);
			_vao.bindIndexBuffer(ibo);
			
			_pointLightShadowMappingProgram->setUniform("u_model", shapeData.matrix);
			
			glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
		}
	}
}

void ShadowMapPass::restorePipelineImpl()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	glm::ivec2 size = getSize();
	glViewport(0, 0, size.x, size.y);
}
