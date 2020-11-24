in vec3 TexCoords;

layout(bindless_sampler) uniform samplerCube skybox;

layout(location = 0) out vec3 color;

void main()
{
	color = texture(skybox, TexCoords).xyz;
}