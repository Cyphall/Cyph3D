in FRAG {
	vec2 TexCoords;
	mat3 TangentToWorld;
	mat3 WorldToTangent;
	vec3 FragPos;
} frag;

uniform vec3 viewPos;

layout(bindless_sampler) uniform sampler2D colorMap;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;
layout(location = 2) out vec4 material;
layout(location = 3) out vec3 geometryNormal;

void main()
{
	color = texture(colorMap, frag.TexCoords).rgb;

	normal = frag.TangentToWorld * vec3(0, 0, 1);
	normal = (normal + 1) * 0.5;

	material = vec4(1, 0, 0, 0);
	
	geometryNormal = frag.TangentToWorld * vec3(0, 0, 1);
	geometryNormal = (geometryNormal + 1) * 0.5;
}