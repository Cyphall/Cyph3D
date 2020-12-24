#include "GeometryPass.h"
#include "../../Window.h"
#include "../../Engine.h"
#include "../../Scene/MeshObject.h"

GeometryPass::GeometryPass(std::unordered_map<std::string, Texture*>& textures):
IRenderPass(textures),
_gbuffer(Engine::getWindow().getSize()),
_normalTexture(TextureCreateInfo
{
   .size = _gbuffer.getSize(),
   .internalFormat = GL_RGB16F
}),
_colorTexture(TextureCreateInfo
{
  .size = _gbuffer.getSize(),
  .internalFormat = GL_RGB16F
}),
_materialTexture(TextureCreateInfo
 {
     .size = _gbuffer.getSize(),
     .internalFormat = GL_RGBA8
 }),
_geometryNormalTexture(TextureCreateInfo
{
   .size = _gbuffer.getSize(),
   .internalFormat = GL_RGB16F
}),
_depthTexture(TextureCreateInfo
{
  .size = _gbuffer.getSize(),
  .internalFormat = GL_DEPTH_COMPONENT24
}),
_objectIndexTexture(TextureCreateInfo
{
  .size = _gbuffer.getSize(),
  .internalFormat = GL_R32I
})
{
	_gbuffer.attach(GL_COLOR_ATTACHMENT0, _normalTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT1, _colorTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT2, _materialTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT3, _geometryNormalTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT4, _objectIndexTexture);
	_gbuffer.attach(GL_DEPTH_ATTACHMENT, _depthTexture);
	
	textures["gbuffer_normal"] = &_normalTexture;
	textures["gbuffer_color"] = &_colorTexture;
	textures["gbuffer_material"] = &_materialTexture;
	textures["gbuffer_gemoetryNormal"] = &_geometryNormalTexture;
	textures["gbuffer_depth"] = &_depthTexture;
	textures["gbuffer_objectIndex"] = &_objectIndexTexture;
	
	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
	_vao.defineFormat(0, 1, 2, GL_FLOAT, offsetof(Mesh::VertexData, uv));
	_vao.defineFormat(0, 2, 3, GL_FLOAT, offsetof(Mesh::VertexData, normal));
	_vao.defineFormat(0, 3, 3, GL_FLOAT, offsetof(Mesh::VertexData, tangent));
}

void GeometryPass::preparePipeline()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void GeometryPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	_gbuffer.clearAll();
	
	int clearIndex[] = {-1, -1, -1, -1};
	glClearNamedFramebufferiv(_gbuffer.getHandle(), GL_COLOR, 4, clearIndex);
	
	_gbuffer.bind();
	_vao.bind();
	
	glm::vec3 pos = camera.position;
	glm::mat4 view = camera.getView();
	glm::mat4 projection = camera.getProjection();
	
	for (int i = 0; i < objects.meshObjects.size(); i++)
	{
		MeshObject* meshObject = objects.meshObjects[i];
		
		const Model* model = meshObject->getModel();
		if (model == nullptr || !model->isResourceReady()) continue;
		
		const Buffer<Mesh::VertexData>& vbo = model->getResource().getVBO();
		const Buffer<GLuint>& ibo = model->getResource().getIBO();
		
		_vao.bindBufferToSlot(vbo, 0);
		_vao.bindIndexBuffer(ibo);
		
		meshObject->getMaterial()->bind(meshObject->getTransform().getWorldMatrix(), view, projection, pos, i);
		
		glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
	}
}

void GeometryPass::restorePipeline()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}
