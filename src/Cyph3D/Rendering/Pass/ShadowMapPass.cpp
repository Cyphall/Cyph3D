#include "ShadowMapPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/Mesh.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/Shape/Shape.h"

ShadowMapPass::ShadowMapPass(glm::uvec2 size):
RenderPass(size, "Shadow map pass"),
_directionalLightShadowMappingProgram({
	{GL_VERTEX_SHADER, "internal/shadow mapping/directional light.vert"}
}),
_pointLightShadowMappingProgram({
	{GL_VERTEX_SHADER, "internal/shadow mapping/point light.vert"},
	{GL_GEOMETRY_SHADER, "internal/shadow mapping/point light.geom"},
	{GL_FRAGMENT_SHADER, "internal/shadow mapping/point light.frag"}
})
{
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
}

ShadowMapPassOutput ShadowMapPass::renderImpl(ShadowMapPassInput& input)
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	
	_vao.bind();
	
	_directionalLightShadowMappingProgram.bind();
	for (DirectionalLight::RenderData& renderData : input.registry.directionalLights)
	{
		if (!renderData.castShadows) continue;
		
		glViewport(0, 0, renderData.mapResolution, renderData.mapResolution);
		
		renderData.shadowMapFramebuffer->bindForDrawing();
		
		float depthColor = 1;
		renderData.shadowMapTexture->clear(&depthColor, GL_DEPTH_COMPONENT, GL_FLOAT);
		
		for (auto& shapeData : input.registry.shapes)
		{
			if (!shapeData.contributeShadows) continue;
			
			if (!shapeData.shape->isReadyForRasterisationRender())
				continue;
			
			const Mesh& mesh = shapeData.shape->getMeshToRender();
			
			const GLBuffer<Mesh::VertexData>& vbo = mesh.getVBO();
			const GLBuffer<GLuint>& ibo = mesh.getIBO();
			_vao.bindBufferToSlot(vbo, 0);
			_vao.bindIndexBuffer(ibo);
			
			glm::mat4 mvp = renderData.lightViewProjection * shapeData.matrix;
			
			_directionalLightShadowMappingProgram.setUniform("u_mvp", mvp);
			
			glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
		}
	}
	
	_pointLightShadowMappingProgram.bind();
	for (PointLight::RenderData& renderData : input.registry.pointLights)
	{
		if (!renderData.castShadows) continue;
		
		glViewport(0, 0, renderData.mapResolution, renderData.mapResolution);
		
		glm::vec3 worldPos = renderData.pos;
		
		renderData.shadowMapFramebuffer->bindForDrawing();
		_pointLightShadowMappingProgram.bind();
		_pointLightShadowMappingProgram.setUniform("u_viewProjections", renderData.viewProjections, 6);
		_pointLightShadowMappingProgram.setUniform("u_lightPos", worldPos);
		_pointLightShadowMappingProgram.setUniform("u_far", renderData.far);
		
		float depthColor = 1;
		renderData.shadowMapTexture->clear(&depthColor, GL_DEPTH_COMPONENT, GL_FLOAT);
		
		for (auto& shapeData : input.registry.shapes)
		{
			if (!shapeData.contributeShadows) continue;
			
			if (!shapeData.shape->isReadyForRasterisationRender())
				continue;
			
			const Mesh& mesh = shapeData.shape->getMeshToRender();
			
			const GLBuffer<Mesh::VertexData>& vbo = mesh.getVBO();
			const GLBuffer<GLuint>& ibo = mesh.getIBO();
			_vao.bindBufferToSlot(vbo, 0);
			_vao.bindIndexBuffer(ibo);
			
			_pointLightShadowMappingProgram.setUniform("u_model", shapeData.matrix);
			
			glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
		}
	}
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	return ShadowMapPassOutput{
	
	};
}