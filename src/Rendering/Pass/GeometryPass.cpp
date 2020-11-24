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
})
{
	_gbuffer.attach(GL_COLOR_ATTACHMENT0, _normalTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT1, _colorTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT2, _materialTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT3, _geometryNormalTexture);
	_gbuffer.attach(GL_DEPTH_ATTACHMENT, _depthTexture);
	
	textures["gbuffer_normal"] = &_normalTexture;
	textures["gbuffer_color"] = &_colorTexture;
	textures["gbuffer_material"] = &_materialTexture;
	textures["gbuffer_gemoetryNormal"] = &_geometryNormalTexture;
	textures["gbuffer_depth"] = &_depthTexture;
}

void GeometryPass::preparePipeline()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void GeometryPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	_gbuffer.clearAll();
	
	_gbuffer.bind();
	
	glm::vec3 pos = camera.position;
	glm::mat4 view = camera.getView();
	glm::mat4 projection = camera.getProjection();
	
	for (MeshObject* meshObject : objects.meshObjects)
	{
		meshObject->render(view, projection, pos);
	}
}

void GeometryPass::restorePipeline()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}
