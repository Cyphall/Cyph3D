#version 460 core
#extension GL_ARB_bindless_texture : enable

in V2F
{
    vec3 fragPos;
    vec2 texCoords;
    vec3 T;
    vec3 N;
} v2f;

uniform vec3 u_viewPos;
uniform int  u_objectIndex;
layout(bindless_sampler) uniform sampler2D u_colorMap;

layout(location = 0) out vec3 o_normal;
layout(location = 1) out vec3 o_color;
layout(location = 2) out vec4 o_material;
layout(location = 3) out vec3 o_geometryNormal;
layout(location = 4) out int  o_objectIndex;

void main()
{
    vec3 T = normalize(v2f.T);
    vec3 N = normalize(v2f.N);
    vec3 B = normalize(cross(v2f.N, v2f.T));
    mat3 tangentToWorld = mat3(T, B, N);
    //mat3 worldToTangent = transpose(tangentToWorld);

    o_color = texture(u_colorMap, v2f.texCoords).rgb;

    o_normal = tangentToWorld * vec3(0, 0, 1);
    o_normal = (o_normal + 1) * 0.5;

    o_material = vec4(1, 0, 0, 0);

    o_objectIndex = u_objectIndex;

    o_geometryNormal = o_normal;
}