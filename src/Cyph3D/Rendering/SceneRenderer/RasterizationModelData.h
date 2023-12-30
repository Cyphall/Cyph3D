#pragma once

#include "Cyph3D/GLSL_types.h"

struct RasterizationModelData
{
	GLSL_mat4 modelMatrix;
	GLSL_mat4 normalMatrix;
	GLSL_DeviceAddress positionVertexBuffer;
	GLSL_DeviceAddress fullVertexBuffer;
	GLSL_DeviceAddress indexBuffer;
	GLSL_int albedoIndex;
	GLSL_int normalIndex;
	GLSL_int roughnessIndex;
	GLSL_int metalnessIndex;
	GLSL_int displacementIndex;
	GLSL_int emissiveIndex;
	GLSL_vec3 albedoValue;
	GLSL_float roughnessValue;
	GLSL_float metalnessValue;
	GLSL_float displacementScale;
	GLSL_float emissiveScale;
};