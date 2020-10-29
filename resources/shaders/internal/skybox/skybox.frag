in vec3 TexCoords;

layout(bindless_sampler) uniform samplerCube skybox;

layout(location = 1) out vec3 color;

void main()
{
	color = texture(skybox, TexCoords).xyz;
}