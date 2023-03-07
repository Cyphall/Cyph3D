#include "GeometryPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/GLObject/Mesh.h"
#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/Shape/MeshShape.h"
#include "Cyph3D/Scene/Camera.h"

#include <glm/gtc/matrix_inverse.hpp>

GeometryPass::GeometryPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size):
RenderPass(textures, size, "Geometry pass"),
_normalTexture(TextureCreateInfo
{
   .size = size,
   .internalFormat = GL_RGBA16F
}),
_colorTexture(TextureCreateInfo
{
  .size = size,
  .internalFormat = GL_RGBA16F
}),
_materialTexture(TextureCreateInfo
 {
     .size = size,
     .internalFormat = GL_RGBA8
 }),
_geometryNormalTexture(TextureCreateInfo
{
   .size = size,
   .internalFormat = GL_RGBA16F
}),
_objectIndexTexture(TextureCreateInfo
{
  .size = size,
  .internalFormat = GL_R32I
}),
_positionTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA32F
}),
_shaderProgram({
	{GL_VERTEX_SHADER, "internal/g-buffer/render to GBuffer.vert"},
	{GL_GEOMETRY_SHADER, "internal/g-buffer/render to GBuffer.geom"},
	{GL_FRAGMENT_SHADER, "internal/g-buffer/render to GBuffer.frag"},
})
{
	_gbuffer.attachColor(0, _normalTexture);
	_gbuffer.attachColor(1, _colorTexture);
	_gbuffer.attachColor(2, _materialTexture);
	_gbuffer.attachColor(3, _geometryNormalTexture);
	_gbuffer.attachColor(4, _objectIndexTexture);
	_gbuffer.attachColor(5, _positionTexture);
	_gbuffer.attachDepth(*textures["z-prepass_depth"]);
	
	_gbuffer.addToDrawBuffers(0, 0);
	_gbuffer.addToDrawBuffers(1, 1);
	_gbuffer.addToDrawBuffers(2, 2);
	_gbuffer.addToDrawBuffers(3, 3);
	_gbuffer.addToDrawBuffers(4, 4);
	_gbuffer.addToDrawBuffers(5, 5);
	
	textures["gbuffer_normal"] = &_normalTexture;
	textures["gbuffer_color"] = &_colorTexture;
	textures["gbuffer_material"] = &_materialTexture;
	textures["gbuffer_gemoetryNormal"] = &_geometryNormalTexture;
	textures["gbuffer_objectIndex"] = &_objectIndexTexture;
	textures["gbuffer_position"] = &_positionTexture;
	
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
	_vao.defineFormat(0, 1, 2, GL_FLOAT, offsetof(Mesh::VertexData, uv));
	_vao.defineFormat(0, 2, 3, GL_FLOAT, offsetof(Mesh::VertexData, normal));
	_vao.defineFormat(0, 3, 3, GL_FLOAT, offsetof(Mesh::VertexData, tangent));
}

void GeometryPass::preparePipelineImpl()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DITHER);
}

void GeometryPass::renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep)
{
	_normalTexture.clear(nullptr, GL_RGBA, GL_UNSIGNED_BYTE);
	_colorTexture.clear(nullptr, GL_RGBA, GL_UNSIGNED_BYTE);
	_materialTexture.clear(nullptr, GL_RGBA, GL_UNSIGNED_BYTE);
	_geometryNormalTexture.clear(nullptr, GL_RGBA, GL_UNSIGNED_BYTE);
	_positionTexture.clear(nullptr, GL_RGBA, GL_FLOAT);
	
	int clearIndex = -1;
	_objectIndexTexture.clear(&clearIndex, GL_RED_INTEGER, GL_INT);
	
	_gbuffer.bindForDrawing();
	_vao.bind();
	
	glm::vec3 pos = camera.getPosition();
	glm::mat4 vp = camera.getProjection() * camera.getView();
	
	_shaderProgram.bind();
	
	for (int i = 0; i < registry.shapes.size(); i++)
	{
		ShapeRenderer::RenderData shapeData = registry.shapes[i];
		
		if (!shapeData.shape->isReadyForRasterisationRender())
			continue;
		
		const Mesh& mesh = shapeData.shape->getMeshToRender();
		
		const GLBuffer<Mesh::VertexData>& vbo = mesh.getVBO();
		const GLBuffer<GLuint>& ibo = mesh.getIBO();

		MaterialAsset* material = shapeData.material;
		if (material == nullptr)
		{
			material = MaterialAsset::getMissingMaterial();
		}
		else if (!material->isLoaded()) // should never happen, but we never know
		{
			material = MaterialAsset::getDefaultMaterial();
		}
		
		_vao.bindBufferToSlot(vbo, 0);
		_vao.bindIndexBuffer(ibo);
		
		_shaderProgram.setUniform("u_albedoMap", material->getAlbedoTexture().getBindlessTextureHandle());
		_shaderProgram.setUniform("u_normalMap", material->getNormalTexture().getBindlessTextureHandle());
		_shaderProgram.setUniform("u_roughnessMap", material->getRoughnessTexture().getBindlessTextureHandle());
		_shaderProgram.setUniform("u_metalnessMap", material->getMetalnessTexture().getBindlessTextureHandle());
		_shaderProgram.setUniform("u_displacementMap", material->getDisplacementTexture().getBindlessTextureHandle());
		_shaderProgram.setUniform("u_emissiveMap", material->getEmissiveTexture().getBindlessTextureHandle());
		
		_shaderProgram.setUniform("u_normalMatrix", glm::inverseTranspose(glm::mat3(shapeData.matrix)));
		_shaderProgram.setUniform("u_model", shapeData.matrix);
		_shaderProgram.setUniform("u_mvp", vp * shapeData.matrix);
		_shaderProgram.setUniform("u_viewPos", pos);
		_shaderProgram.setUniform("u_objectIndex", i);
		
		glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
	}
}

void GeometryPass::restorePipelineImpl()
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DITHER);
}