#include "GeometryPass.h"
#include "../../Window.h"
#include "../../Engine.h"
#include "../../GLObject/Mesh.h"
#include "../Shape/MeshShape.h"

GeometryPass::GeometryPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size):
RenderPass(textures, size, "Geometry pass"),
_gbuffer(size),
_normalTexture(TextureCreateInfo
{
   .size = size,
   .internalFormat = GL_RGB16F
}),
_colorTexture(TextureCreateInfo
{
  .size = size,
  .internalFormat = GL_RGB16F
}),
_materialTexture(TextureCreateInfo
 {
     .size = size,
     .internalFormat = GL_RGBA8
 }),
_geometryNormalTexture(TextureCreateInfo
{
   .size = size,
   .internalFormat = GL_RGB16F
}),
_objectIndexTexture(TextureCreateInfo
{
  .size = size,
  .internalFormat = GL_R32I
}),
_positionTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGB32F
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

void GeometryPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera)
{
	_normalTexture.clear(GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	_colorTexture.clear(GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	_materialTexture.clear(GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	_geometryNormalTexture.clear(GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	_positionTexture.clear(GL_RGB, GL_FLOAT, nullptr);
	
	int clearIndex = -1;
	_objectIndexTexture.clear(GL_RED_INTEGER, GL_INT, &clearIndex);
	
	_gbuffer.bindForDrawing();
	_vao.bind();
	
	glm::vec3 pos = camera.getPosition();
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
		
		shapeData.material->bind(shapeData.matrix, vp, pos, i);
		
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
