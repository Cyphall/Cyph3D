#include "LightingPass.h"

#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/Mesh.h"
#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/Shape/Shape.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"

#include <glm/gtc/matrix_inverse.hpp>

LightingPass::LightingPass(glm::uvec2 size):
	RenderPass(size, "Lighting pass"),
	_pointLightsBuffer(GL_DYNAMIC_DRAW),
	_directionalLightsBuffer(GL_DYNAMIC_DRAW),
	_shader({
		{GL_VERTEX_SHADER, "internal/lighting/lighting.vert"},
		{GL_FRAGMENT_SHADER, "internal/lighting/lighting.frag"},
	}),
	_rawRenderTexture(TextureCreateInfo
	{
	 .size = size,
	 .internalFormat = GL_RGBA16F
	}),
	_objectIndexTexture(TextureCreateInfo
	{
		.size = size,
		.internalFormat = GL_R32I
	})
{
	_framebuffer.attachColor(0, _rawRenderTexture);
	_framebuffer.attachColor(1, _objectIndexTexture);

	_framebuffer.addToDrawBuffers(0, 0);
	_framebuffer.addToDrawBuffers(1, 1);

	_vao.defineFormat(0, 0, 3, GL_FLOAT, offsetof(Mesh::VertexData, position));
	_vao.defineFormat(0, 1, 2, GL_FLOAT, offsetof(Mesh::VertexData, uv));
	_vao.defineFormat(0, 2, 3, GL_FLOAT, offsetof(Mesh::VertexData, normal));
	_vao.defineFormat(0, 3, 3, GL_FLOAT, offsetof(Mesh::VertexData, tangent));
}

LightingPassOutput LightingPass::renderImpl(LightingPassInput& input)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	
	std::vector<GLSL_DirectionalLight> directionalLightData;
	directionalLightData.reserve(input.registry.directionalLights.size());
	for (DirectionalLight::RenderData& renderData : input.registry.directionalLights)
	{
		GLSL_DirectionalLight data{};
		data.fragToLightDirection = renderData.fragToLightDirection;
		data.intensity = renderData.intensity;
		data.color = renderData.color;
		data.castShadows = renderData.castShadows;
		if (renderData.castShadows)
		{
			data.lightViewProjection = renderData.lightViewProjection;
			data.shadowMap = renderData.shadowMapTexture->getBindlessTextureHandle();
			data.mapSize = renderData.mapSize;
			data.mapDepth = renderData.mapDepth;
		}
		directionalLightData.push_back(data);
	}
	_directionalLightsBuffer.resize(directionalLightData.size());
	_directionalLightsBuffer.setData(directionalLightData);
	_directionalLightsBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 1);

	std::vector<GLSL_PointLight> pointLightData;
	pointLightData.reserve(input.registry.pointLights.size());
	for (PointLight::RenderData& renderData : input.registry.pointLights)
	{
		GLSL_PointLight data{};
		data.pos = renderData.pos;
		data.intensity = renderData.intensity;
		data.color = renderData.color;
		data.castShadows = renderData.castShadows;
		if (renderData.castShadows)
		{
			data.shadowMap = renderData.shadowMapTexture->getBindlessTextureHandle();
			data.far = renderData.far;
			data.maxTexelSizeAtUnitDistance = 2.0f / renderData.mapResolution;
		}
		pointLightData.push_back(data);
	}
	_pointLightsBuffer.resize(pointLightData.size());
	_pointLightsBuffer.setData(pointLightData);
	_pointLightsBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);

	_rawRenderTexture.clear(nullptr, GL_RGBA, GL_HALF_FLOAT);
	
	int clearIndex = -1;
	_objectIndexTexture.clear(&clearIndex, GL_RED_INTEGER, GL_INT);
	
	_framebuffer.attachDepth(input.depth);
	_framebuffer.bindForDrawing();
	_vao.bind();

	_shader.setUniform("u_viewProjectionInv", glm::inverse(input.camera.getProjection() * input.camera.getView()));
	_shader.setUniform("u_viewPos", input.camera.getPosition());

	glm::mat4 vp = input.camera.getProjection() * input.camera.getView();
	
	_shader.bind();
	
	for (int i = 0; i < input.registry.shapes.size(); i++)
	{
		ShapeRenderer::RenderData shapeData = input.registry.shapes[i];
		
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

		_shader.setUniform("u_normalMatrix", glm::inverseTranspose(glm::mat3(shapeData.matrix)));
		_shader.setUniform("u_model", shapeData.matrix);
		_shader.setUniform("u_mvp", vp * shapeData.matrix);
		
		_shader.setUniform("u_albedoMap", material->getAlbedoTexture().getBindlessTextureHandle());
		_shader.setUniform("u_normalMap", material->getNormalTexture().getBindlessTextureHandle());
		_shader.setUniform("u_roughnessMap", material->getRoughnessTexture().getBindlessTextureHandle());
		_shader.setUniform("u_metalnessMap", material->getMetalnessTexture().getBindlessTextureHandle());
		_shader.setUniform("u_displacementMap", material->getDisplacementTexture().getBindlessTextureHandle());
		_shader.setUniform("u_emissiveMap", material->getEmissiveTexture().getBindlessTextureHandle());

		_shader.setUniform("u_objectIndex", i);
		
		glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
	}
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DITHER);
	
	return LightingPassOutput{
		.rawRender = _rawRenderTexture,
		.objectIndex = _objectIndexTexture
	};
}